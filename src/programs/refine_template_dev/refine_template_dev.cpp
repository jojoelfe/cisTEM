#include "../../core/core_headers.h"
#include "./refine_template_dev.h"

class
        RefineTemplateDevApp : public MyApp {
  public:
    bool DoCalculation( );
    void DoInteractiveUserInput( );

  private:
};

// FIXME why do we define classes inside other classes, this seems to be a bad practice
class TemplateComparisonObject {
  public:
    Image *          input_reconstruction, *windowed_particle, *projection_filter;
    AnglesAndShifts* angles;
    float            pixel_size_factor;
    //	int							slice = 1;
};

// This is the function which will be minimized
Peak TemplateScore(void* scoring_parameters) {
    TemplateComparisonObject* comparison_object = reinterpret_cast<TemplateComparisonObject*>(scoring_parameters);
    Image                     current_projection;
    //	Peak box_peak;

    current_projection.Allocate(comparison_object->projection_filter->logical_x_dimension, comparison_object->projection_filter->logical_x_dimension, false);
    if ( comparison_object->input_reconstruction->logical_x_dimension != current_projection.logical_x_dimension ) {
        Image padded_projection;
        padded_projection.Allocate(comparison_object->input_reconstruction->logical_x_dimension, comparison_object->input_reconstruction->logical_x_dimension, false);
        comparison_object->input_reconstruction->ExtractSlice(padded_projection, *comparison_object->angles, 1.0f, false);
        padded_projection.SwapRealSpaceQuadrants( );
        padded_projection.BackwardFFT( );
        padded_projection.ChangePixelSize(&current_projection, comparison_object->pixel_size_factor, 0.001f, true);
        //		padded_projection.ChangePixelSize(&padded_projection, comparison_object->pixel_size_factor, 0.001f);
        //		padded_projection.ClipInto(&current_projection);
        //		current_projection.ForwardFFT();
    }
    else {
        comparison_object->input_reconstruction->ExtractSlice(current_projection, *comparison_object->angles, 1.0f, false);
        current_projection.SwapRealSpaceQuadrants( );
        current_projection.BackwardFFT( );
        current_projection.ChangePixelSize(&current_projection, comparison_object->pixel_size_factor, 0.001f, true);
    }

    //	current_projection.QuickAndDirtyWriteSlice("projections.mrc", comparison_object->slice);
    //	comparison_object->slice++;
    current_projection.MultiplyPixelWise(*comparison_object->projection_filter);
    //	current_projection.BackwardFFT();
    //	current_projection.AddConstant(-current_projection.ReturnAverageOfRealValuesOnEdges());
    //	current_projection.Resize(comparison_object->windowed_particle->logical_x_dimension, comparison_object->windowed_particle->logical_y_dimension, 1, 0.0f);
    //	current_projection.ForwardFFT();
    current_projection.ZeroCentralPixel( );
    current_projection.DivideByConstant(sqrtf(current_projection.ReturnSumOfSquares( )));
#ifdef MKL
    // Use the MKL
    vmcMulByConj(current_projection.real_memory_allocated / 2, reinterpret_cast<MKL_Complex8*>(comparison_object->windowed_particle->complex_values), reinterpret_cast<MKL_Complex8*>(current_projection.complex_values), reinterpret_cast<MKL_Complex8*>(current_projection.complex_values), VML_EP | VML_FTZDAZ_ON | VML_ERRMODE_IGNORE);
#else
    for ( long pixel_counter = 0; pixel_counter < current_projection.real_memory_allocated / 2; pixel_counter++ ) {
        current_projection.complex_values[pixel_counter] = std::conj(current_projection.complex_values[pixel_counter]) * comparison_object->windowed_particle->complex_values[pixel_counter];
    }
#endif
    current_projection.BackwardFFT( );
    //	wxPrintf("ping");

    return current_projection.FindPeakWithIntegerCoordinates( );
    //	box_peak = current_projection.FindPeakWithIntegerCoordinates();
    //	wxPrintf("address = %li\n", box_peak.physical_address_within_image);
    //	box_peak.x = 0.0f;
    //	box_peak.y = 0.0f;
    //	box_peak.value = current_projection.real_values[33152];
    //	return box_peak;
}

IMPLEMENT_APP(RefineTemplateDevApp)

// override the DoInteractiveUserInput

void RefineTemplateDevApp::DoInteractiveUserInput( ) {
    using namespace cistem::refine_template_arguments;

    RefineTemplateArguments args;
    args.GetInteractiveInputs( );
#ifdef _OPENMP
    // max_threads = my_input->GetIntFromUser("Max. threads to use for calculation", "When threading, what is the max threads to run", "1", 1);
#else
    args.set(max_threads, 1);
#endif
    args.set_manual_arguments(my_current_job);
}

// override the do calculation method which will be what is actually run..

bool RefineTemplateDevApp::DoCalculation( ) {
    using namespace cistem::refine_template_arguments;
    wxDateTime start_time = wxDateTime::Now( );

    RefineTemplateArguments args;
    args.get_from_current_job(my_current_job);

    return true;
}
