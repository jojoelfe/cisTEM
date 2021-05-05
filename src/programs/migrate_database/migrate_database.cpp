#include "../../core/core_headers.h"

class
MigrateDatabase : public MyApp
{

	public:

	bool DoCalculation();
	void DoInteractiveUserInput();

	private:

    Project current_project;
    bool MigrateProject(wxString old_project_directory, wxString new_project_directory);
};



IMPLEMENT_APP(MigrateDatabase)

// override the DoInteractiveUserInput

void MigrateDatabase::DoInteractiveUserInput()
{

	int new_z_size = 1;

	UserInput *my_input = new UserInput("AddTwoStacks", 1.0);

	std::string input_filename_database	=		my_input->GetFilenameFromUser("Database filename", "Filename of the database", "project.db", true );
	

	delete my_input;

	my_current_job.Reset(1);
	my_current_job.ManualSetArguments("t", input_filename_database.c_str());

}

// override the do calculation method which will be what is actually run..

bool MigrateDatabase::DoCalculation()
{
	std::string	input_filename_database						= my_current_job.arguments[0].ReturnStringArgument();
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



		// has the database file been moved?  If so, attempt to convert it..

		if (current_project.database.database_file.GetPath() != current_project.project_directory.GetFullPath())
		{
			// database has moved?

			wxPrintf(wxString::Format("It looks like this project has been moved :-\n\nCurrent Dir. \t: %s\nStored Dir. \t: %s\n", current_project.database.database_file.GetPath(), current_project.project_directory.GetFullPath()));
			

				if (MigrateProject(current_project.project_directory.GetFullPath(), current_project.database.database_file.GetPath()) == false)
				{
					// something went wrong

					wxPrintf( "Something went wrong!");
					current_project.Close(false);
					return false;
				}
				else
				{
					// close and reopen
					current_project.Close(false);
					if (current_project.OpenProjectFromFile(project_database) == false) return false;
				}

			


		} else {

            wxPrintf("Database appears to be in the right path. Doing nothing. \n");
        }


		
		
	}
	else
	{
		MyPrintWithDetails("An error occured opening the database file..");
	}
	return true;
}

bool MigrateDatabase::MigrateProject(wxString old_project_directory, wxString new_project_directory)
{
	// this is very boring.. go through and update all the links in the database..
	// start transaction

	current_project.database.Begin();

	// Master settings..
	current_project.database.ExecuteSQL(wxString::Format("UPDATE MASTER_SETTINGS SET PROJECT_DIRECTORY = '%s';", new_project_directory).ToUTF8().data());

	// Movie Assets

	current_project.database.ExecuteSQL(wxString::Format("UPDATE MOVIE_ASSETS SET FILENAME = REPLACE(FILENAME, '%s', '%s');", old_project_directory, new_project_directory).ToUTF8().data());

	// Image Assets

	current_project.database.ExecuteSQL(wxString::Format("UPDATE IMAGE_ASSETS SET FILENAME = REPLACE(FILENAME, '%s', '%s');", old_project_directory, new_project_directory).ToUTF8().data());

	// Volume Assets

	current_project.database.ExecuteSQL(wxString::Format("UPDATE VOLUME_ASSETS SET FILENAME = REPLACE(FILENAME, '%s', '%s');", old_project_directory, new_project_directory).ToUTF8().data());

	// Refinement Package Assets

	current_project.database.ExecuteSQL(wxString::Format("UPDATE REFINEMENT_PACKAGE_ASSETS SET STACK_FILENAME = REPLACE(STACK_FILENAME, '%s', '%s');", old_project_directory, new_project_directory).ToUTF8().data());

	// Movie alignment list

	current_project.database.ExecuteSQL(wxString::Format("UPDATE MOVIE_ALIGNMENT_LIST SET OUTPUT_FILE = REPLACE(OUTPUT_FILE, '%s', '%s');", old_project_directory, new_project_directory).ToUTF8().data());

	// Estimated CTF Parameters

	current_project.database.ExecuteSQL(wxString::Format("UPDATE ESTIMATED_CTF_PARAMETERS SET OUTPUT_DIAGNOSTIC_FILE = REPLACE(OUTPUT_DIAGNOSTIC_FILE, '%s', '%s');", old_project_directory, new_project_directory).ToUTF8().data());

	// Classification List

	current_project.database.ExecuteSQL(wxString::Format("UPDATE CLASSIFICATION_LIST SET CLASS_AVERAGE_FILE = REPLACE(CLASS_AVERAGE_FILE, '%s', '%s');", old_project_directory, new_project_directory).ToUTF8().data());

	// Commit

	// Template Matching...

	current_project.database.ExecuteSQL(wxString::Format("UPDATE TEMPLATE_MATCH_LIST SET MIP_OUTPUT_FILE = REPLACE(MIP_OUTPUT_FILE, '%s', '%s');", old_project_directory, new_project_directory).ToUTF8().data());
	current_project.database.ExecuteSQL(wxString::Format("UPDATE TEMPLATE_MATCH_LIST SET SCALED_MIP_OUTPUT_FILE = REPLACE(SCALED_MIP_OUTPUT_FILE, '%s', '%s');", old_project_directory, new_project_directory).ToUTF8().data());
	current_project.database.ExecuteSQL(wxString::Format("UPDATE TEMPLATE_MATCH_LIST SET PSI_OUTPUT_FILE = REPLACE(PSI_OUTPUT_FILE, '%s', '%s');", old_project_directory, new_project_directory).ToUTF8().data());
	current_project.database.ExecuteSQL(wxString::Format("UPDATE TEMPLATE_MATCH_LIST SET THETA_OUTPUT_FILE = REPLACE(THETA_OUTPUT_FILE, '%s', '%s');", old_project_directory, new_project_directory).ToUTF8().data());
	current_project.database.ExecuteSQL(wxString::Format("UPDATE TEMPLATE_MATCH_LIST SET PHI_OUTPUT_FILE = REPLACE(PHI_OUTPUT_FILE, '%s', '%s');", old_project_directory, new_project_directory).ToUTF8().data());
	current_project.database.ExecuteSQL(wxString::Format("UPDATE TEMPLATE_MATCH_LIST SET DEFOCUS_OUTPUT_FILE = REPLACE(DEFOCUS_OUTPUT_FILE, '%s', '%s');", old_project_directory, new_project_directory).ToUTF8().data());
	current_project.database.ExecuteSQL(wxString::Format("UPDATE TEMPLATE_MATCH_LIST SET PIXEL_SIZE_OUTPUT_FILE = REPLACE(PIXEL_SIZE_OUTPUT_FILE, '%s', '%s');", old_project_directory, new_project_directory).ToUTF8().data());
	current_project.database.ExecuteSQL(wxString::Format("UPDATE TEMPLATE_MATCH_LIST SET HISTOGRAM_OUTPUT_FILE = REPLACE(HISTOGRAM_OUTPUT_FILE, '%s', '%s');", old_project_directory, new_project_directory).ToUTF8().data());
	current_project.database.ExecuteSQL(wxString::Format("UPDATE TEMPLATE_MATCH_LIST SET PROJECTION_RESULT_OUTPUT_FILE = REPLACE(PROJECTION_RESULT_OUTPUT_FILE, '%s', '%s');", old_project_directory, new_project_directory).ToUTF8().data());

	current_project.database.Commit();

	// everything should be ok?

	return true;
}
