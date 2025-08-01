/*  ML Object Annotation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Class to store the annotation manually done on an object in an image or video
 */

 #include "Common/Cpp/BitmapConversion.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "ML_ObjectAnnotation.h"


namespace PokemonAutomation{
namespace ML{


ObjectAnnotation::ObjectAnnotation(): user_box(0,0,0,0), mask_box(0,0,0,0) {}

// if failed to pass, will throw JsonParseException
ObjectAnnotation ObjectAnnotation::from_json(const JsonValue& json){
    ObjectAnnotation anno_obj;

    const JsonObject& json_obj = json.to_object_throw();
    const JsonArray& user_box_array = json_obj.get_array_throw("UserBox");
    anno_obj.user_box = ImagePixelBox(
        size_t(user_box_array[0].to_integer_throw()),
        size_t(user_box_array[1].to_integer_throw()),
        size_t(user_box_array[2].to_integer_throw()),
        size_t(user_box_array[3].to_integer_throw())
    );
    const JsonArray& mask_box_array = json_obj.get_array_throw("MaskBox");
    anno_obj.mask_box = ImagePixelBox(
        size_t(mask_box_array[0].to_integer_throw()),
        size_t(mask_box_array[1].to_integer_throw()),
        size_t(mask_box_array[2].to_integer_throw()),
        size_t(mask_box_array[3].to_integer_throw())
    );
    const size_t mask_width = anno_obj.mask_box.width(), mask_height = anno_obj.mask_box.height();
    const size_t num_mask_ele = mask_width * mask_height;
    
    const std::string mask_base64 = json_obj.get_string_throw("Mask");
    anno_obj.mask = unpack_bit_vector_from_base64(mask_base64, num_mask_ele);
    if (anno_obj.mask.size() != num_mask_ele){
        std::string err_msg = "wrong decoded object annotation mask size: decoded " + std::to_string(anno_obj.mask.size())
            + " but should be " + std::to_string(num_mask_ele);
        throw ParseException(err_msg);
    }

    anno_obj.label = json_obj.get_string_throw("Label");
    
    return anno_obj;
}

JsonValue ObjectAnnotation::to_json() const{
    JsonObject json_obj;
    JsonArray user_box_arr;
    user_box_arr.push_back(int64_t(user_box.min_x));
    user_box_arr.push_back(int64_t(user_box.min_y));
    user_box_arr.push_back(int64_t(user_box.max_x));
    user_box_arr.push_back(int64_t(user_box.max_y));
    json_obj["UserBox"] = std::move(user_box_arr);

    JsonArray mask_box_arr;
    mask_box_arr.push_back(int64_t(mask_box.min_x));
    mask_box_arr.push_back(int64_t(mask_box.min_y));
    mask_box_arr.push_back(int64_t(mask_box.max_x));
    mask_box_arr.push_back(int64_t(mask_box.max_y));
    json_obj["MaskBox"] = std::move(mask_box_arr);

    json_obj["Mask"] = pack_bit_vector_to_base64(mask);

    json_obj["Label"] = label;

    return json_obj;
}


}
}