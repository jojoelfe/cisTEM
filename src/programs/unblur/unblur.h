#ifndef __PROGRAMS_UNBLUR_H__
#define __PROGRAMS_UNBLUR_H__
#import "../../core/arguments.h"

enum args : int {
    input_filename = 0,
    max_threads,
    whiten_image,
    arguments_length // Must stay on the bottom
};

input_filename.c_str( ),
        output_filename.c_str( ),
        original_pixel_size,
        minimum_shift_in_angstroms,
        maximum_shift_in_angstroms,
        should_dose_filter,
        should_restore_power,
        termination_threshold_in_angstroms,
        max_iterations,
        bfactor_in_angstroms,
        should_mask_central_cross,
        horizontal_mask_size,
        vertical_mask_size,
        acceleration_voltage,
        exposure_per_frame,
        pre_exposure_amount,
        movie_is_gain_corrected,
        gain_filename.ToStdString( ).c_str( ),
        movie_is_dark_corrected,
        dark_filename.ToStdString( ).c_str( ),
        output_binning_factor,
        correct_mag_distortion,
        mag_distortion_angle,
        mag_distortion_major_scale,
        mag_distortion_minor_scale,
        write_out_amplitude_spectrum,
        amplitude_spectrum_filename.c_str( ),
        write_out_small_sum_image,
        small_sum_image_filename.c_str( ),
        first_frame,
        last_frame,
        number_of_frames_for_running_average,
        max_threads,
        save_aligned_frames,
        aligned_frames_filename.c_str( ),
        output_shift_text_file.c_str( ),
        eer_frames_per_image,
        eer_super_res_factor

        arguments::ArgumentDescription UnblurArgumentsDescription[arguments_length] = {
                {arguments::FilenameArgument,
                 "Input starfile",
                 std::string("input.star")},
                {arguments::IntegerArgument,
                 "Maximum number of threads to use",
                 1},
                {arguments::BooleanArgument,
                 "Should I whiten the image?",
                 false}};

arguments::Arguments test_args = arguments::Arguments<arguments_length>(TestArgumentsDescription, "TestProgram", 1.00);

#endif