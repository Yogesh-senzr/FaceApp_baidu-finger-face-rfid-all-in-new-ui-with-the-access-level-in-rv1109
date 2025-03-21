#pragma once
#include <interface/bface_types.h>

namespace bface {
/**
 * @deprecated The old type FeatureId_t use float vector to store binary data, which may cause misunderstanding.
 * @brief initialize face feature library with input feature array
 * @param[in] feature_lib an array of face features, see FeatureId_t
 * @param[in] type store type, only support rgb, nir, each type can only have one lib at most
 * @return BFACE_SUCCESS on SDK intialization success, otherwise an error code
 */
BFACE_API
BFACE_DEPRECATED("The old 'FeatureId_t' type is prone to misunderstandings about feature data types, please use 'RawFeatureId_t' to relpace it!")
BFACE_STATUS bface_feature_lib_init(const std::vector<FeatureId_t>& feature_lib, StoreType type = RGB_FEATURE);

/**
 * @brief initialize face feature library with input feature array
 * @param[in] feature_lib an array of face features, see RawFeatureId_t
 * @param[in] type store type, only support rgb, nir, each type can only have one lib at most
 * @return BFACE_SUCCESS on SDK intialization success, otherwise an error code
 */
BFACE_API
BFACE_STATUS bface_feature_lib_init(const std::vector<RawFeatureId_t>& feature_lib, StoreType type = RGB_FEATURE);

/**
 * @deprecated The feature variable use float vector to store binary data, which may cause misunderstanding.
 * @brief bface_search_feature takes one face feature TOP number and search threshold, and search the most similar N result
 * @param[in] face_feature one face feature
 * @param[in] search_num TOP number of search
 * @param[in] type store type, only support rgb, nir
 * @param[out] search_result_ptr the output search result
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_DEPRECATED("The old feature variable type 'std::vector<float>' may cause misunderstanding, please use 'std::vector<Byte_t>' to relpace it!")
BFACE_STATUS bface_search_feature(
        const std::vector<float>& face_feature, 
        int search_num,
        std::vector<SearchResult_t>* search_result_ptr, 
        StoreType type = RGB_FEATURE
);

/**
 * @brief bface_search_feature takes one face feature TOP number and search threshold, and search the most similar N result
 * @param[in] face_feature one face feature
 * @param[in] search_num TOP number of search
 * @param[in] type store type, only support rgb, nir
 * @param[out] search_result_ptr the output search result
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_search_feature(
        const std::vector<Byte_t>& face_feature, 
        int search_num, 
        std::vector<SearchResult_t>* search_result_ptr, 
        StoreType type = RGB_FEATURE
);

/**
 * @deprecated The old type FeatureId_t use float vector to store binary data, which may cause misunderstanding.
 * @brief add a face feature to Bface feature search engine
 * @param[in] face_feature face feature with feature id
 * @param[in] type store type, only support rgb, nir
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_DEPRECATED("The old 'FeatureId_t' type may cause misunderstandings about feature data types, please use 'RawFeatureId_t' to relpace it!")
BFACE_STATUS bface_add_feature(const FeatureId_t& face_feature, StoreType type = RGB_FEATURE);

/**
 * @brief add a face feature to Bface feature search engine
 * @param[in] face_feature face feature with feature id
 * @param[in] type store type, only support rgb, nir
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_add_feature(const RawFeatureId_t& face_feature, StoreType type = RGB_FEATURE);

/**
 * @brief delete a feature record from Bface feature search engine
 * @param[in] feature_id id of face feature
 * @param[in] type store type, only support rgb, nir
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_remove_feature(int feature_id, StoreType type = RGB_FEATURE);

/**
 * @brief find a face feature according to feature id
 * @param[in] feature_id id of face feature
 * @param[in] type tore type, only support rgb, nir ,rgb mask, rgb id
 * @param[out] face_feature feature in library
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_get_feature(const int feature_id, std::vector<Byte_t>* face_feature, StoreType type = RGB_FEATURE);

/**
 * @brief clear all feature from Bface feature search engine
 * @param[in] type store type, only support rgb, nir
 * @return API status code.
 */
BFACE_API
BFACE_STATUS bface_clear_feature(StoreType type = RGB_FEATURE);

/**
 * @brief get feature library size from Bface feature search engine
 * @param[in] type store type, only support rgb, nir
 * @param[out] size feature library size
 * @return API status code.
 */
BFACE_API
BFACE_STATUS bface_get_feature_lib_size(int* size, StoreType type = RGB_FEATURE);

/**
 * @deprecated The old api use float pointer to specify the input feature data, which may cause misunderstanding.
 * @brief compute the confidence of the similarity of two input features
 * @param[in] feat1 the first input features
 * @param[in] feat2 the second input features
 * @param[in] len the length of input features
 * @param[out] conf_ptr pointer to the output confidence
 * @param[in] type store type, only support rgb, nir
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 * @note application should make sure the length of feat1 is equal to the length of feat2.
 */
BFACE_API
BFACE_DEPRECATED("This old API may cause misunderstandings about feature data type, please use the parameters list like: 'const std::vector<Byte_t>& feature_1, const std::vector<Byte_t>& feature_2, float* conf_ptr, StoreType type = RGB_FEATURE' to call the new overloaded interface relpace it!")
BFACE_STATUS bface_compare_features(const float* feat1, const float* feat2, int len, float* conf_ptr, StoreType type = RGB_FEATURE);

/**
 * @brief Calculate the similarity score of the two input features.
 * @param[in] feature_1 the first input feature.
 * @param[in] feature_2 the second input feature.
 * @param[out] conf_ptr pointer to the output score.
 * @param[in] type store type, support RGB_FEATURE, NIR_FEATURE or RGB_ID_FEATURE.
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 * @note application should make sure the length of feature_1 is equal to the length of feature_2.
 */
BFACE_API
BFACE_STATUS bface_compare_features(
        const std::vector<Byte_t>& feature_1, 
        const std::vector<Byte_t>& feature_2, 
        float* conf_ptr, 
        StoreType type = RGB_FEATURE
);

/**
 * @brief Decode feature from cloud server in standard Base64 rule (Text-to-Binary).  
 * @param[in] str Pointer to the input encoded feature data(string-type). 
 * @param[in] len Length of the input data(unit: Byte). 
 * @param[out] feature Origin feature data(binary-type).
 * @return API status code, BFACE_SUCCESS indicates no error occurred.
 */
BFACE_API
BFACE_STATUS bface_base64_decode(const char* str, const int len, std::vector<Byte_t>& feature);

} /// namespace bface