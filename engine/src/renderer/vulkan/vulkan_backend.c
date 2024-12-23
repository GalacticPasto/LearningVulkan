#include "vulkan_backend.h"
#include "containers/darray.h"
#include "core/dmemory.h"
#include "core/dstring.h"
#include "core/logger.h"
#include "vulkan_command_buffer.h"
#include "vulkan_device.h"
#include "vulkan_platform.h"
#include "vulkan_renderpass.h"
#include "vulkan_swapchain.h"
#include "vulkan_types.h"

static vulkan_context context = {};

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                 const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData);

void vulkan_instance_destroy();
void vk_create_command_buffers();

b8 vulkan_initialize(renderer_backend *backend, const char *application_name, struct platform_state *plat_state)
{
    context.frame_buffer_width  = 800;
    context.frame_buffer_height = 600;

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
    platform_get_specific_surface_extensions(&required_extensions);   // platform specific extensions

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

    u32 logSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT; //|
                                                                    //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};

    debugCreateInfo.messageSeverity = logSeverity;
    debugCreateInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugCreateInfo.pfnUserCallback = vk_debug_callback;

    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.vk_instance, "vkCreateDebugUtilsMessengerEXT");

    DASSERT_MSG(func, "Failed to create debug messenger!");

    VK_CHECK(func(context.vk_instance, &debugCreateInfo, 0, &context.debug_messenger));

    DDEBUG("Vulkan debugger created.");
#endif

    // get vulkan platform specific surface
    if (!platform_create_vk_surface(plat_state, &context))
    {
        DERROR("Vulkan surface creation failed");
        return false;
    }

    // create physical device
    if (!vk_create_device(&context))
    {
        DERROR("Vulkan device creation failed");
        return false;
    }

    // create swapchain
    if (!vk_create_swapchain(&context, context.frame_buffer_width, context.frame_buffer_height))
    {
        DERROR("Swapchain creation failed");
        return false;
    }

    if (!vk_create_renderpass(&context))
    {
        DERROR("Renderpass creation failed");
        return false;
    }

    vk_create_command_buffers();

    DINFO("VULKAN initialized");
    return true;
}

void vulkan_shutdown(struct renderer_backend *backend)
{

    DDEBUG("Destroying command buffers...");
    for (i32 i = 0; i < context.vk_swapchain.image_count; i++)
    {
        vk_command_buffer_free(&context, context.vk_device.graphics_command_pool, &context.graphics_command_buffers[i]);
    }
    darray_destroy(context.graphics_command_buffers);
    context.graphics_command_buffers = 0;

    DDEBUG("Destroying command pools...");
    vkDestroyCommandPool(context.vk_device.logical, context.vk_device.graphics_command_pool, 0);

    DDEBUG("Destroying renderpass...");
    DASSERT(vk_destroy_renderpass(&context));

    DDEBUG("Destroying swapchain...");
    DASSERT(vk_destroy_swapchain(&context));

    DDEBUG("Destroying vulkan device...");
    DASSERT(vk_destroy_device(&context));

    DDEBUG("Destroying vulkan surface...");
    vkDestroySurfaceKHR(context.vk_instance, context.vk_surface, 0);

    DDEBUG("Destroying vulkan debugMessenger...");
    if (context.debug_messenger)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT func =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.vk_instance, "vkDestroyDebugUtilsMessengerEXT");
        func(context.vk_instance, context.debug_messenger, 0);
    }

    DDEBUG("Destroying vulkan instance...");
    vkDestroyInstance(context.vk_instance, 0);
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

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                 const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData)
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

void vk_create_command_buffers()
{
    if (!context.graphics_command_buffers)
    {
        context.graphics_command_buffers = darray_reserve(vulkan_command_buffer, context.vk_swapchain.image_count);

        for (i32 i = 0; i < context.vk_swapchain.image_count; i++)
        {
            dzero_memory(&context.graphics_command_buffers[i], sizeof(vulkan_command_buffer));
        }
    }

    for (i32 i = 0; i < context.vk_swapchain.image_count; i++)
    {
        vk_command_buffer_allocate(&context, context.vk_device.graphics_command_pool, &context.graphics_command_buffers[i]);
    }
    DINFO("Vulkan Command buffers created");
}
