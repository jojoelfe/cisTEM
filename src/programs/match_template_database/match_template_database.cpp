#include "../../core/core_headers.h"

class
MatchTemplateDatabase : public MyApp
{

	public:

	bool DoCalculation();
	void DoInteractiveUserInput();

	private:

    Project current_project;
	AssetGroup active_group;
	bool StartEstimation();
	float resolution_limit;
	float orientations_per_process;
	float current_orientation_counter;

	int job_counter;
	int number_of_rotations = 0;
	int number_of_defocus_positions;
	int number_of_pixel_size_positions;

	bool use_gpu;
	int  max_threads = 1; // Only used for the GPU code. For GUI this comes from the run profile -> command line override as in other programs.

	int image_number_for_gui;
	int number_of_jobs_per_image_in_gui;
	int number_of_jobs;

	double voltage_kV;
	double spherical_aberration_mm;
	double amplitude_contrast;
	double defocus1;
	double defocus2;
	double defocus_angle;
	double phase_shift;
	double iciness;
};



IMPLEMENT_APP(MatchTemplateDatabase)

// override the DoInteractiveUserInput

void MatchTemplateDatabase::DoInteractiveUserInput()
{


	bool 		use_gpu_input = false;
	int			max_threads = 1; // Only used for the GPU code

	UserInput *my_input = new UserInput("MatchTemplateDatabase", 1.0);

	std::string input_filename_database	=		my_input->GetFilenameFromUser("Database filename", "Filename of the database", "project.db", true );
	std::string input_image_group	=		my_input->GetStringFromUser("Name of image group", "Name of the group of images that should be searched", "all", true );
	int input_volume_asset	=		my_input->GetIntFromUser("ID of Volume Asset", "ID of the volume asset that should be used to search", "1", true );
	float high_resolution_limit = my_input->GetFloatFromUser("High resolution limit (A)", "High resolution limit of the data used for alignment in Angstroms", "8.0", 0.0);
	float angular_step = my_input->GetFloatFromUser("Out of plane angular step (0.0 = set automatically)", "Angular step size for global grid search", "0.0", 0.0);
	float in_plane_angular_step = my_input->GetFloatFromUser("In plane angular step (0.0 = set automatically)", "Angular step size for in-plane rotations during the search", "0.0", 0.0);
//	best_parameters_to_keep = my_input->GetIntFromUser("Number of top hits to refine", "The number of best global search orientations to refine locally", "20", 1);
	float defocus_search_range = my_input->GetFloatFromUser("Defocus search range (A)", "Search range (-value ... + value) around current defocus", "500.0", 0.0);
	float defocus_step = my_input->GetFloatFromUser("Defocus step (A) (0.0 = no search)", "Step size used in the defocus search", "50.0", 0.0);
	float pixel_size_search_range = my_input->GetFloatFromUser("Pixel size search range (A)", "Search range (-value ... + value) around current pixel size", "0.1", 0.0);
	float pixel_size_step = my_input->GetFloatFromUser("Pixel size step (A) (0.0 = no search)", "Step size used in the pixel size search", "0.01", 0.0);
	float padding = my_input->GetFloatFromUser("Padding factor", "Factor determining how much the input volume is padded to improve projections", "1.0", 1.0, 2.0);
//	ctf_refinement = my_input->GetYesNoFromUser("Refine defocus", "Should the particle defocus be refined?", "No");
	float particle_radius_angstroms = my_input->GetFloatFromUser("Mask radius for global search (A) (0.0 = max)", "Radius of a circular mask to be applied to the input images during global search", "0.0", 0.0);
	wxString my_symmetry = my_input->GetSymmetryFromUser("Template symmetry", "The symmetry of the template reconstruction", "C1");
#ifdef ENABLEGPU
	use_gpu_input = my_input->GetYesNoFromUser("Use GPU", "Offload expensive calcs to GPU","No");
	max_threads = my_input->GetIntFromUser("Max. threads to use for calculation", "when threading, what is the max threads to run", "1", 1);
#endif

	delete my_input;

	my_current_job.Reset(3);
	my_current_job.ManualSetArguments("ttit", input_filename_database.c_str(),
															input_image_group,
															input_volume_asset,
															high_resolution_limit,
															my_symmetry.ToUTF8().data(),
															angular_step,
															in_plane_angular_step,													
															use_gpu_input,
															max_threads);

}

// override the do calculation method which will be what is actually run..

bool MatchTemplateDatabase::DoCalculation()
{
	std::string	input_filename_database						= my_current_job.arguments[0].ReturnStringArgument();
	std::string input_image_group	=  my_current_job.arguments[1].ReturnStringArgument();
	int input_volume_asset =  my_current_job.arguments[2].ReturnStringArgument();
    wxFileName database_file;
    database_file.Assign(input_filename_database);       
    database_file.MakeAbsolute();
	wxString project_database = database_file.GetFullPath();

    wxPrintf(wxString::Format("Attempting migration of %s\n", project_database));

	if (wxDirExists(project_database + ".lock"))
	{
		wxPrintf("Database is locked. Exiting.\n");
        return false;
	}
    if (current_project.OpenProjectFromFile(project_database) == true)
	{
		// check this project is not "locked"

		long my_process_id = wxGetProcessId();
		wxString my_hostname = wxGetFullHostName();

		long database_process_id;
		wxString database_hostname;

		if (current_project.database.DoesTableExist("PROCESS_LOCK") == true)
		{
			current_project.database.ReturnProcessLockInfo(database_process_id, database_hostname);

			if (my_process_id != 0 && database_process_id != -1 && my_process_id != database_process_id)
			{
				wxPrintf("Database is locked. Exiting.\n");
                return false;

			}
		}

		// DO DATABASE VERSION CHECK HERE!


		if (current_project.integer_database_version > INTEGER_DATABASE_VERSION)
		{
			wxPrintf("This database was created in a newer version of cisTEM, and cannot be opened.");
			current_project.Close(false);
			return false;
		}
		else
		{
			// need to upgrade the database here.
		}



		// start the matching here...
		StartEstimation(input_image_group, input_volume_asset);

		
		
	}
	else
	{
		MyPrintWithDetails("An error occured opening the database file..");
	}
	return true;
}

bool MatchTemplateDatabase::StartEstimation(std::string input_image_group, int input_volume_asset)
{

	active_group.CopyFrom(&image_asset_panel->all_groups_list->groups[GroupComboBox->GetSelection()]);
	current_project.database.BeginAllImageGroupsSelect();
	AssetGroup temp_group;
	while (main_frame->current_project.database.last_return_code == SQLITE_ROW)
	{
		temp_group = main_frame->current_project.database.GetNextImageGroup();

		// the members of this group are referenced by asset id's, we need to translate this to array position..

		if (temp_group.name == input_image_group) {
			active_group.CopyFrom(temp_group);
		}

		
	}
	main_frame->current_project.database.EndAllImageGroupsSelect();

	

	

	// Package the job details..

	EulerSearch	*current_image_euler_search;
	ImageAsset *current_image;
	VolumeAsset *current_volume;

	current_volume = 





	
	ref_box_size_in_pixels = current_volume->x_size / current_volume->pixel_size;

	ParameterMap parameter_map;
	parameter_map.SetAllTrue();

	float wanted_out_of_plane_angular_step = OutofPlaneStepNumericCtrl->ReturnValue();
	float wanted_in_plane_angular_step = InPlaneStepNumericCtrl->ReturnValue();

	float defocus_search_range;
	float defocus_step;
	float pixel_size_search_range;
	float pixel_size_step;

	if (DefocusSearchYesRadio->GetValue() == true)
	{
		defocus_search_range = DefocusSearchRangeNumericCtrl->ReturnValue();
		defocus_step = DefocusSearchStepNumericCtrl->ReturnValue();
	}
	else
	{
		defocus_search_range = 0.0f;
		defocus_step = 0.0f;
	}

	if (PixelSizeSearchYesRadio->GetValue() == true)
	{

		pixel_size_search_range = PixelSizeSearchRangeNumericCtrl->ReturnValue();
		pixel_size_step = PixelSizeSearchStepNumericCtrl->ReturnValue();
	}
	else
	{
		pixel_size_search_range = 0.0f;
		pixel_size_step = 0.0f;
	}

	float min_peak_radius = MinPeakRadiusNumericCtrl->ReturnValue();

	if (UseGpuCheckBox->GetValue() == true)
	{
		use_gpu = true;
	}
	else
	{
		use_gpu = false;
	}

	wxString wanted_symmetry = SymmetryComboBox->GetValue();
	wanted_symmetry = SymmetryComboBox->GetValue().Upper();
	float high_resolution_limit = HighResolutionLimitNumericCtrl->ReturnValue();

	wxPrintf("\n\nWanted symmetry %s, Defocus Range %3.3f, Defocus Step %3.3f\n",wanted_symmetry,defocus_search_range,defocus_step);

	RunProfile active_refinement_run_profile = run_profiles_panel->run_profile_manager.run_profiles[RunProfileComboBox->GetSelection()];

	int number_of_processes = active_refinement_run_profile.ReturnTotalJobs();

	// how many jobs are there going to be..

	// get first image to make decisions about how many jobs.. .we assume this is representative.


	current_image = image_asset_panel->ReturnAssetPointer(active_group.members[0]);
	current_image_euler_search = new EulerSearch;
	// WARNING: resolution_limit below is used before its value is set
	current_image_euler_search->InitGrid(wanted_symmetry, wanted_out_of_plane_angular_step, 0.0, 0.0, 360.0, wanted_in_plane_angular_step, 0.0, current_image->pixel_size / resolution_limit, parameter_map, 1);

	if (wanted_symmetry.StartsWith("C1"))
	{
	if (current_image_euler_search->test_mirror == true) // otherwise the theta max is set to 90.0 and test_mirror is set to true.  However, I don't want to have to test the mirrors.
	{
		current_image_euler_search->theta_max = 180.0f;
	}
	}

	current_image_euler_search->CalculateGridSearchPositions(false);

	if (use_gpu)
	{
		//	number_of_jobs_per_image_in_gui = std::max((int)1,number_of_processes / 2); // Using two threads in each job
		number_of_jobs_per_image_in_gui = number_of_processes; // Using two threads in each job

		number_of_jobs =  number_of_jobs_per_image_in_gui * active_group.number_of_members;

		wxPrintf("In USEGPU:\n There are %d search positions\nThere are %d jobs per image\n", current_image_euler_search->number_of_search_positions, number_of_jobs_per_image_in_gui);
		delete current_image_euler_search;
	}
	else
	{
		if (active_group.number_of_members >= 5 || current_image_euler_search->number_of_search_positions < number_of_processes * 20) number_of_jobs_per_image_in_gui = number_of_processes;
		else
		if (current_image_euler_search->number_of_search_positions > number_of_processes * 250) number_of_jobs_per_image_in_gui = number_of_processes * 10;
		else number_of_jobs_per_image_in_gui = number_of_processes * 5;

		number_of_jobs = number_of_jobs_per_image_in_gui * active_group.number_of_members;

		delete current_image_euler_search;
	}

// Some settings for testing
//	float defocus_search_range = 1200.0f;
//	float defocus_step = 200.0f;

	// number of rotations

	for (float current_psi = 0.0f; current_psi <= 360.0f; current_psi += wanted_in_plane_angular_step)
	{
		number_of_rotations++;
	}

	current_job_package.Reset(active_refinement_run_profile, "match_template", number_of_jobs);

	expected_number_of_results = 0;
	number_of_received_results = 0;

	// loop over all images..

	OneSecondProgressDialog *my_progress_dialog = new OneSecondProgressDialog ("Preparing Job", "Preparing Job...", active_group.number_of_members, this, wxPD_REMAINING_TIME | wxPD_AUTO_HIDE| wxPD_APP_MODAL);

	TemplateMatchJobResults temp_result;
	temp_result.input_job_id = -1;
	temp_result.job_type = TEMPLATE_MATCH_FULL_SEARCH;
	temp_result.mask_radius = 0.0f;
	temp_result.min_peak_radius = min_peak_radius;
	temp_result.exclude_above_xy_threshold = false;
	temp_result.xy_change_threshold = 0.0f;

	for (int image_counter = 0; image_counter < active_group.number_of_members; image_counter++)
	{
		image_number_for_gui = image_counter + 1;

		// current image asset

		current_image = image_asset_panel->ReturnAssetPointer(active_group.members[image_counter]);

		// setup the euler search for this image..
		// this needs to be changed when more parameters are added.
		// right now, the resolution is always Nyquist.

		resolution_limit = current_image->pixel_size * 2.0f;
		current_image_euler_search = new EulerSearch;
		current_image_euler_search->InitGrid(wanted_symmetry, wanted_out_of_plane_angular_step, 0.0, 0.0, 360.0, wanted_in_plane_angular_step, 0.0, current_image->pixel_size / resolution_limit, parameter_map, 1);
		if (wanted_symmetry.StartsWith("C1"))
		{
			if (current_image_euler_search->test_mirror == true) // otherwise the theta max is set to 90.0 and test_mirror is set to true.  However, I don't want to have to test the mirrors.
			{
				current_image_euler_search->theta_max = 180.0f;
			}
		}
		current_image_euler_search->CalculateGridSearchPositions(false);

		if (DefocusSearchYesRadio->GetValue() == true) number_of_defocus_positions = 2 * myround(float(defocus_search_range)/float(defocus_step)) + 1;
		else number_of_defocus_positions = 1;

		if (PixelSizeSearchYesRadio->GetValue() == true) number_of_pixel_size_positions = 2 * myround(float(pixel_size_search_range)/float(pixel_size_step)) + 1;
		else number_of_pixel_size_positions = 1;

		wxPrintf("For Image %li\nThere are %i search positions\nThere are %i jobs per image\n", active_group.members[image_counter],current_image_euler_search->number_of_search_positions, number_of_jobs_per_image_in_gui);
		wxPrintf("Calculating %i correlation maps\n", current_image_euler_search->number_of_search_positions * number_of_rotations * number_of_defocus_positions * number_of_pixel_size_positions);
		// how many orientations will each process do for this image..
		expected_number_of_results += current_image_euler_search->number_of_search_positions * number_of_rotations * number_of_defocus_positions * number_of_pixel_size_positions;
		orientations_per_process = float(current_image_euler_search->number_of_search_positions) / float(number_of_jobs_per_image_in_gui);
		if (orientations_per_process < 1) orientations_per_process = 1;

		int number_of_previous_template_matches =  main_frame->current_project.database.ReturnNumberOfPreviousTemplateMatchesByAssetID(current_image->asset_id);
		main_frame->current_project.database.GetCTFParameters(current_image->ctf_estimation_id,voltage_kV,spherical_aberration_mm,amplitude_contrast,defocus1,defocus2,defocus_angle,phase_shift, iciness);

		wxString mip_output_file = main_frame->current_project.template_matching_asset_directory.GetFullPath();
		mip_output_file += wxString::Format("/%s_mip_%i_%i.mrc", current_image->filename.GetName(), current_image->asset_id, number_of_previous_template_matches);

		wxString best_psi_output_file = main_frame->current_project.template_matching_asset_directory.GetFullPath();
		best_psi_output_file += wxString::Format("/%s_psi_%i_%i.mrc", current_image->filename.GetName(), current_image->asset_id, number_of_previous_template_matches);

		wxString best_theta_output_file = main_frame->current_project.template_matching_asset_directory.GetFullPath();
		best_theta_output_file += wxString::Format("/%s_theta_%i_%i.mrc", current_image->filename.GetName(), current_image->asset_id, number_of_previous_template_matches);

		wxString best_phi_output_file = main_frame->current_project.template_matching_asset_directory.GetFullPath();
		best_phi_output_file += wxString::Format("/%s_phi_%i_%i.mrc", current_image->filename.GetName(), current_image->asset_id, number_of_previous_template_matches);


		wxString best_defocus_output_file = main_frame->current_project.template_matching_asset_directory.GetFullPath();
		best_defocus_output_file += wxString::Format("/%s_defocus_%i_%i.mrc", current_image->filename.GetName(), current_image->asset_id, number_of_previous_template_matches);

		wxString best_pixel_size_output_file = main_frame->current_project.template_matching_asset_directory.GetFullPath();
		best_pixel_size_output_file += wxString::Format("/%s_pixel_size_%i_%i.mrc", current_image->filename.GetName(), current_image->asset_id, number_of_previous_template_matches);

		wxString scaled_mip_output_file = main_frame->current_project.template_matching_asset_directory.GetFullPath();
		scaled_mip_output_file += wxString::Format("/%s_scaled_mip_%i_%i.mrc", current_image->filename.GetName(), current_image->asset_id, number_of_previous_template_matches);

		wxString output_histogram_file = main_frame->current_project.template_matching_asset_directory.GetFullPath();
		output_histogram_file += wxString::Format("/%s_histogram_%i_%i.txt", current_image->filename.GetName(), current_image->asset_id, number_of_previous_template_matches);

		wxString output_result_file = main_frame->current_project.template_matching_asset_directory.GetFullPath();
		output_result_file += wxString::Format("/%s_plotted_result_%i_%i.mrc", current_image->filename.GetName(), current_image->asset_id, number_of_previous_template_matches);

		wxString correlation_avg_output_file = main_frame->current_project.template_matching_asset_directory.GetFullPath();
		correlation_avg_output_file += wxString::Format("/%s_avg_%i_%i.mrc", current_image->filename.GetName(), current_image->asset_id, number_of_previous_template_matches);

		wxString correlation_std_output_file = main_frame->current_project.template_matching_asset_directory.GetFullPath();
		correlation_std_output_file += wxString::Format("/%s_std_%i_%i.mrc", current_image->filename.GetName(), current_image->asset_id, number_of_previous_template_matches);

//		wxString correlation_std_output_file = "/dev/null";
		current_orientation_counter = 0;

		wxString 	input_search_image = current_image->filename.GetFullPath();
		wxString 	input_reconstruction = current_volume->filename.GetFullPath();
		float		pixel_size = current_image->pixel_size;

		input_image_filenames.Add(input_search_image);

		float low_resolution_limit = 300.0f; // FIXME set this somehwere that is not buried in the code!

		temp_result.image_asset_id = current_image->asset_id;
		temp_result.job_name = wxString::Format("Full search with %s", current_volume->filename.GetName());
		temp_result.ref_volume_asset_id = current_volume->asset_id;
		wxDateTime now = wxDateTime::Now();
		temp_result.datetime_of_run = (long int) now.GetAsDOS();
		temp_result.symmetry = wanted_symmetry;
		temp_result.pixel_size = pixel_size;
		temp_result.voltage = voltage_kV;
		temp_result.spherical_aberration = spherical_aberration_mm;
		temp_result.amplitude_contrast = amplitude_contrast;
		temp_result.defocus1 = defocus1;
		temp_result.defocus2 = defocus2;
		temp_result.defocus_angle = defocus_angle;
		temp_result.phase_shift = phase_shift;
		temp_result.low_res_limit = low_resolution_limit;
		temp_result.high_res_limit = high_resolution_limit;
		temp_result.out_of_plane_step = wanted_out_of_plane_angular_step;
		temp_result.in_plane_step = wanted_in_plane_angular_step;
		temp_result.defocus_search_range = defocus_search_range;
		temp_result.defocus_step = defocus_step;
		temp_result.pixel_size_search_range = pixel_size_search_range;
		temp_result.pixel_size_step = pixel_size_step;
		temp_result.reference_box_size_in_angstroms = ref_box_size_in_pixels * pixel_size;
		temp_result.mip_filename = mip_output_file;
		temp_result.scaled_mip_filename = scaled_mip_output_file;
		temp_result.psi_filename = best_psi_output_file;
		temp_result.theta_filename = best_theta_output_file;
		temp_result.phi_filename = best_phi_output_file;
		temp_result.defocus_filename = best_defocus_output_file;
		temp_result.pixel_size_filename = best_pixel_size_output_file;
		temp_result.histogram_filename = output_histogram_file;
		temp_result.projection_result_filename = output_result_file;
		temp_result.avg_filename = correlation_avg_output_file;
		temp_result.std_filename = correlation_std_output_file;

		cached_results.Add(temp_result);

		for (job_counter = 0; job_counter < number_of_jobs_per_image_in_gui; job_counter++)
		{


//			float high_resolution_limit = resolution_limit;
			int best_parameters_to_keep = 1;
//			float defocus_search_range = 0.0f;
//			float defocus_step = 0.0f;
			float padding = 1;
			bool ctf_refinement = false;
			float mask_radius_search = 0.0f; //current_volume->x_size; // this is actually not really used...

			wxPrintf("\n\tFor image %i, current_orientation_counter is %f\n",image_number_for_gui,current_orientation_counter);
			if (current_orientation_counter >= current_image_euler_search->number_of_search_positions) current_orientation_counter = current_image_euler_search->number_of_search_positions - 1;
			int first_search_position = myroundint(current_orientation_counter);
			current_orientation_counter += orientations_per_process;
			if (current_orientation_counter >= current_image_euler_search->number_of_search_positions || job_counter == number_of_jobs_per_image_in_gui - 1) current_orientation_counter = current_image_euler_search->number_of_search_positions - 1;
			int last_search_position = myroundint(current_orientation_counter);
			current_orientation_counter++;

			wxString directory_for_results = main_frame->current_project.image_asset_directory.GetFullPath();
//			wxString directory_for_results = main_frame->ReturnScratchDirectory();


			//wxPrintf("%i = %i - %i\n", job_counter, first_search_position, last_search_position);


			current_job_package.AddJob("ttffffffffffifffffbfftttttttttftiiiitttfbi",	input_search_image.ToUTF8().data(),
																	input_reconstruction.ToUTF8().data(),
																	pixel_size,
																	voltage_kV,
																	spherical_aberration_mm,
																	amplitude_contrast,
																	defocus1,
																	defocus2,
																	defocus_angle,
																	low_resolution_limit,
																	high_resolution_limit,
																	wanted_out_of_plane_angular_step,
																	best_parameters_to_keep,
																	defocus_search_range,
																	defocus_step,
																	pixel_size_search_range,
																	pixel_size_step,
																	padding,
																	ctf_refinement,
																	mask_radius_search,
																	phase_shift,
																	mip_output_file.ToUTF8().data(),
																	best_psi_output_file.ToUTF8().data(),
																	best_theta_output_file.ToUTF8().data(),
																	best_phi_output_file.ToUTF8().data(),
																	best_defocus_output_file.ToUTF8().data(),
																	best_pixel_size_output_file.ToUTF8().data(),
																	scaled_mip_output_file.ToUTF8().data(),
																	correlation_avg_output_file.ToUTF8().data(),
																	wanted_symmetry.ToUTF8().data(),
																	wanted_in_plane_angular_step,
																	output_histogram_file.ToUTF8().data(),
																	first_search_position,
																	last_search_position,
																	image_number_for_gui,
																	number_of_jobs_per_image_in_gui,
																	correlation_std_output_file.ToUTF8().data(),
																	directory_for_results.ToUTF8().data(),
																	output_result_file.ToUTF8().data(),
																	min_peak_radius,
																	use_gpu,
																	max_threads);
		}

		delete current_image_euler_search;
		my_progress_dialog->Update(image_counter + 1);
	}


	my_progress_dialog->Destroy();

	// Get ID's from database for writing results as they come in..

	template_match_id = main_frame->current_project.database.ReturnHighestTemplateMatchID() + 1;
	template_match_job_id =  main_frame->current_project.database.ReturnHighestTemplateMatchJobID() + 1;

	// launch a controller

	my_job_id = main_frame->job_controller.AddJob(this, run_profiles_panel->run_profile_manager.run_profiles[RunProfileComboBox->GetSelection()].manager_command, run_profiles_panel->run_profile_manager.run_profiles[RunProfileComboBox->GetSelection()].gui_address);

	if (my_job_id != -1)
	{
		SetNumberConnectedTextToZeroAndStartTracking();

		StartPanel->Show(false);
		ProgressPanel->Show(true);
		InputPanel->Show(false);

		ExpertPanel->Show(false);
		InfoPanel->Show(false);
		OutputTextPanel->Show(true);
		ResultsPanel->Show(true);

		GroupComboBox->Enable(false);
		Layout();
	}


	ProgressBar->Pulse();
}

void MatchTemplatePanel::HandleSocketTemplateMatchResultReady(wxSocketBase *connected_socket, int &image_number, float &threshold_used, ArrayOfTemplateMatchFoundPeakInfos &peak_infos, ArrayOfTemplateMatchFoundPeakInfos &peak_changes)
{
	// result is available for an image..

	cached_results[image_number - 1].found_peaks.Clear();
	cached_results[image_number - 1].found_peaks = peak_infos;
	cached_results[image_number - 1].used_threshold = threshold_used;

	ResultsPanel->SetActiveResult(cached_results[image_number - 1]);

	// write to database..

	main_frame->current_project.database.Begin();

	cached_results[image_number - 1].job_id = template_match_job_id;
	main_frame->current_project.database.AddTemplateMatchingResult(template_match_id, cached_results[image_number - 1]);
	template_match_id++;

	main_frame->current_project.database.SetActiveTemplateMatchJobForGivenImageAssetID(cached_results[image_number - 1].image_asset_id, template_match_job_id);
	main_frame->current_project.database.Commit();
	match_template_results_panel->is_dirty = true;
}