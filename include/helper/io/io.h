#ifndef _HELPER_IO_IO_H
#define _HELPER_IO_IO_H
#include <interface/bface_types.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>
namespace bface {
/**
 * @brief all io operations are integrated into io namespace under bface
 */
namespace helper {
/**
 * @brief Parse an environment variable into a set of path strings
 * @details this function read linux environment variable PATH, and parse it's value, append
 * it to a vector of paths
 *
 * @param[in] env environment variable
 * @param[in] ptr_array_paths ptr to vector where append result
 * @return true if find valide path or false
 * @note if env is empty or no env environment variable, this function does nothing.
 */
BFACE_API
bool parse_path_from_env_path(const std::string& env, std::vector<std::string>* ptr_array_paths);

/**
 * @brief search a file in a set of given paths
 * @details this function searches a file in a set of given paths, and returns absolute path
 *  if founded, or it return empty string
 *
 * @param[in] paths       a set of given paths
 * @param[in] filename    searching filename, without path
 * @param[in] sub_folder_name  sub_folder name in root path
 * @return  return the first founded absolute path, if not found, return empty string
 * @note to speed up the searching process, try to put your real path in the paths head
 * @see parse_path_from_env_path
 */
BFACE_API
std::string search_file_from_paths(const std::vector<std::string> &paths, \
    const std::string &filename, const std::string& sub_folder_name = "");

/**
 * @brief check a given file, folder or link exists or not
 *
 * @param[in] abs_path file path to check
 * @return true if given file or folder exists, or false
 * @note the provided abs_path must be a absolute path.
 */
BFACE_API
bool is_file_exist(const std::string &abs_path);

/**
 * @brief given a detection string, this function parses it into a set a bounding boxes
 * @param[in] detection_str a detection string, organized as CSV format, the following:
 *  frame_name, number_of_bounding_boxes, xmin1, ymin1, xmax1, ymax1, conf1, xmin2, ymin2, xmax2, ymax2, conf2,
 * @param[out] bboxes output a set of bounding boxes.
 * @return true if successfully, or false
 */
BFACE_API
bool parse_one_frame_detections_from_string(const std::string& detection_str, std::vector<BoundingBox_t>* bboxes);

/**
 * @brief given a tracking string, this function parses it into a set a tracked bounding boxes
 * @param[in] tracking_str the input tracking string, organized as CSV format, the following:
 *  frame_name, track_cnt, face_id_1, track_state_1, xmin_1, ymin_1, xmax_1, ymax_1, ...
 * @param tracked_bboxes output a set of tracked boxes.
 * @return true if no error happens.
 */
BFACE_API
bool parse_one_frame_tracking_from_string(const std::string& tracking_str, std::vector<TrackedBox_t>* tracked_bboxes);

/**
 * @brief get the absolute path of current executable program
 * @return absolute path
 */
BFACE_API
std::string get_exe_abs_path();

/**
 * @brief walk through directory and return all file path in the directory or subdirectory
 * @param[in] dir_path directory path 
 * @param file_paths output file paths 
 * @return true if no error happens.
 */
BFACE_API
bool walk_directory(const std::string& dir_path, std::vector<std::string>* file_paths);

/**
 * @brief check if filepath is directory
 * @return true if filepath is directory
 */
BFACE_API
bool is_directory(const std::string& filepath);

/**
 * @brief split file extension and basename of file path/name
 * @param[in] file_path path of the file or filename
 * @param ext output extension
 * @param base_name output base file path/name without extension
 * @return true if no error happens.
 */
BFACE_API
bool split_file_extension(const std::string& file_path, std::string* ext, std::string* base_name);

/**
 * @brief get filename frome file path
 * @param[in] file_path path of the file
 * @param dir_path directory path of the file
 * @return true if no error happens.
 */
BFACE_API
bool get_directory(const std::string& file_path, std::string* dir_path);

/**
 * @brief get filename frome file path
 * @param[in] file_path path of the file
 * @param filename filename of the file
 * @return true if no error happens.
 */
BFACE_API
bool get_filename(const std::string& file_path, std::string* filename);

/**
 * @brief list file names in the directory
 * @param[in] dir_path path of input dir
 * @param file_names output file names
 * @return true if no error happens.
 */
BFACE_API
bool list_directory(const std::string& dir_path, std::vector<std::string>* file_names);

/**
 * @brief create the directory
 * @param[in] dir_name input directory name
 * @return true if no error happens.
 */
BFACE_API
bool create_directory(const std::string dir_name);

/**
 * @brief create the multi directory. 
 * @param[in] dir_name input directory name
 * @return true if no error happens.
 */
BFACE_API
bool create_dirs(const std::string dir_name);

}   /// namespace helper
}   /// namespace bface
#endif  // _HELPER_IO_IO_H
