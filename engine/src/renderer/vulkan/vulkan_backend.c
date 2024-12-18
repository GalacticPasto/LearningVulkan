#include "vulkan_backend.h"
#include "containers/darray.h"
#include "core/dstring.h"
#include "core/logger.h"
#include "vulkan_platform.h"
#include "vulkan_types.h"

static vulkan_context context = {};

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                 VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
                                                 const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
                                                 void                                       *userData);

b8 vulkan_initialize(struct renderer_backend *backend, const char *application_name, struct platform_state *platSate)
{

    // typedef VkResult (VKAPI_PTR *PFN_vkCreateInstance)(const VkInstanceCreateInfo* pCreateInfo, const
    // VkAllocationCallbacks* pAllocator, VkInstance* pInstance);
    //  initialize a vk instance
    VkApplicationInfo app_info = {};

    app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName   = application_name;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName        = "learning_vulkan";
    app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion         = VK_API_VERSION_1_0;

    // layers

    VkInstanceCreateInfo instance_info = {};
    context.instance_info              = &instance_info;

    instance_info.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pNext            = 0;
    instance_info.flags            = 0;
    instance_info.pApplicationInfo = &app_info;

    // array of const char pointers
    const char **required_layers = darray_create(const char *);
    darray_push(required_layers, &"VK_LAYER_KHRONOS_validation");
    u32 required_layers_length = darray_length(required_layers);

#ifdef _DEBUG
    // validation layers
    u32 layer_count = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&layer_count, 0));
    DDEBUG("Available layer counts %d", layer_count);

    VkLayerProperties *vk_layer_properties = (VkLayerProperties *)darray_reserve(VkLayerProperties, layer_count);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&layer_count, vk_layer_properties));

    // check for required extensions
    for (int i = 0; i < required_layers_length; i++)
    {
        b8 found = false;
        for (int j = 0; j < layer_count; j++)
        {
            if (string_compare(required_layers[i], vk_layer_properties[j].layerName))
            {
                found = true;
                DDEBUG("Found %s", required_layers[i]);
                break;
            }
        }
        if (!found)
        {
            DERROR("Validation layer '%s' is missing", required_layers[i]);
            return false;
        }
    }

    DDEBUG("All required validation layers are found");
    darray_destroy(vk_layer_properties);
#endif

    instance_info.enabledLayerCount   = required_layers_length;
    instance_info.ppEnabledLayerNames = required_layers;

    // Extensions

    const char **required_extensions = darray_create(const char *);
    darray_push(required_extensions, &VK_KHR_SURFACE_EXTENSION_NAME); // Generic surface extension
    platform_get_specific_surface_extensions(&required_extensions);

#ifdef _DEBUG

    darray_push(required_extensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // debug utilities
    DDEBUG("Required extensions:");
    u32 length = darray_length(required_extensions);
    for (u32 i = 0; i < length; ++i)
    {
        DDEBUG(required_extensions[i]);
    }
#endif

    instance_info.enabledExtensionCount   = darray_length(required_extensions);
    instance_info.ppEnabledExtensionNames = required_extensions;

    VK_CHECK(vkCreateInstance(&instance_info, 0, &context.vk_instance));
    DINFO("Succesfully created vulkan instance");

#ifdef _DEBUG

    DDEBUG("Creating Vulkan debugger");

    u32 logSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT; //|
                                                      //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};

    debugCreateInfo.messageSeverity = logSeverity;
    debugCreateInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugCreateInfo.pfnUserCallback = vk_debug_callback;

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        context.vk_instance, "vkCreateDebugUtilsMessengerEXT");

    DASSERT_MSG(func, "Failed to create debug messenger!");

    VK_CHECK(func(context.vk_instance, &debugCreateInfo, 0, &context.debug_messenger));

    DDEBUG("Vulkan debugger created.");
#endif

    return true;
}

void vulkan_shutdown(struct renderer_backend *backend)
{
}

void vulkan_resize(struct renderer_backend *backend, u16 width, u16 height)
{
}

b8 vulkan_begin_frame(struct renderer_backend *backend, f32 delta_time)
{

    return false;
}

b8 vulkan_end_frame(struct renderer_backend *backend, f32 delta_time)
{
    return false;
}

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                 VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
                                                 const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
                                                 void                                       *userData)
{
    switch (messageSeverity)
    {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            DERROR(callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            DWARN(callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            DINFO(callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            DTRACE(callbackData->pMessage);
            break;
    }
    return VK_FALSE;
}
