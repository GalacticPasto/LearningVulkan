#include "vulkan_device.h"
#include "containers/darray.h"
#include "core/dmemory.h"
#include "core/dstring.h"
#include "core/logger.h"

typedef struct vulkan_physical_device_requirements
{
    b8 graphics_queue;
    b8 compute_queue;
    b8 transfer_queue;
    b8 present_queue;
    b8 discrete_gpu;
    // darray of requirements
    char **required_device_extensions;

    vulkan_swapchain_support_info *swapchain_info;

} vulkan_physical_device_requirements;

typedef struct vulkan_physical_device_queue_family_info
{
    u32 graphics_family_index;
    u32 present_family_index;
    u32 transfer_family_index;
    u32 compute_family_index;

} vulkan_physical_device_queue_family_info;

static b8   select_physical_device(vulkan_context *context);
static b8   is_device_suitable(const VkSurfaceKHR surface, const vulkan_physical_device_requirements *requirements,
                               vulkan_physical_device_queue_family_info *queue_info, VkPhysicalDevice device,
                               const VkPhysicalDeviceProperties *properties, const VkPhysicalDeviceFeatures *features,
                               vulkan_swapchain_support_info *out_swapchain_info);
static void populate_buf(char *queue_types_buf, VkQueueFlags queueFlags);
static void vk_query_swapchain_support_info(VkPhysicalDevice device, VkSurfaceKHR surface, vulkan_swapchain_support_info *out_swapchain_info);

b8 vk_create_device(vulkan_context *context)
{
    if (!select_physical_device(context))
    {
        return false;
    }
    // create queues
    u32 no_of_queues = 1;

    b8 transfer_queue_shares_graphics_queue = context->vk_device.graphics_queue_index == context->vk_device.transfer_queue_index;

    b8 present_queue_shares_graphics_queue = context->vk_device.graphics_queue_index == context->vk_device.present_queue_index;

    if (!transfer_queue_shares_graphics_queue)
    {
        no_of_queues++;
    }
    if (!present_queue_shares_graphics_queue)
    {
        no_of_queues++;
    }

    u32 queue_indicies[no_of_queues];
    u32 index               = 0;
    queue_indicies[index++] = context->vk_device.graphics_queue_index;

    if (!transfer_queue_shares_graphics_queue)
    {
        queue_indicies[index++] = context->vk_device.transfer_queue_index;
    }
    if (!present_queue_shares_graphics_queue)
    {
        queue_indicies[index++] = context->vk_device.present_queue_index;
    }

    VkDeviceQueueCreateInfo queue_create_infos[no_of_queues];
    for (i32 i = 0; i < no_of_queues; i++)
    {
        queue_create_infos[i].pNext            = 0;
        queue_create_infos[i].flags            = 0;
        queue_create_infos[i].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = queue_indicies[i];
        queue_create_infos[i].queueCount       = 1;
        f32 queue_priority                     = 1.0f;
        queue_create_infos[i].pQueuePriorities = &queue_priority;
    }

    const char **required_device_extensions = darray_create(const char *);
    darray_push(required_device_extensions, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // create the logical device

    VkPhysicalDeviceFeatures device_features_to_be_enabled = {};

    VkDeviceCreateInfo device_create_info      = {};
    device_create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount    = no_of_queues;
    device_create_info.pQueueCreateInfos       = queue_create_infos;
    device_create_info.enabledExtensionCount   = darray_length(required_device_extensions);
    device_create_info.ppEnabledExtensionNames = required_device_extensions;
    device_create_info.pEnabledFeatures        = 0;

    VK_CHECK(vkCreateDevice(context->vk_device.physical, &device_create_info, NULL, &context->vk_device.logical));
    DINFO("Created vulkan logical device");

    // get the queues
    vkGetDeviceQueue(context->vk_device.logical, context->vk_device.graphics_queue_index, 0, &context->vk_device.graphics_queue);
    vkGetDeviceQueue(context->vk_device.logical, context->vk_device.transfer_queue_index, 0, &context->vk_device.transfer_queue);
    vkGetDeviceQueue(context->vk_device.logical, context->vk_device.present_queue_index, 0, &context->vk_device.present_queue);

    return true;
}

b8 select_physical_device(vulkan_context *context)
{

    u32 device_count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(context->vk_instance, &device_count, 0));

    if (device_count == 0)
    {
        DFATAL("No GPU's found with vulkan support!!");
        return false;
    }

    VkPhysicalDevice physical_devices[device_count];
    VK_CHECK(vkEnumeratePhysicalDevices(context->vk_instance, &device_count, physical_devices));

    vulkan_physical_device_requirements requirements = {};
    // for now we only have these requirements
    requirements.discrete_gpu   = false;
    requirements.graphics_queue = true;
    requirements.compute_queue  = true;
    requirements.transfer_queue = true;
    requirements.present_queue  = true;

    char **required_device_extensions = darray_create(char *);
    darray_push(required_device_extensions, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    requirements.required_device_extensions = required_device_extensions;

    for (i32 i = 0; i < device_count; i++)
    {

        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(physical_devices[i], &device_properties);

        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceFeatures(physical_devices[i], &device_features);

        VkPhysicalDeviceMemoryProperties device_memory;
        vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &device_memory);

        vulkan_physical_device_queue_family_info queue_info = {};

        b8 result = is_device_suitable(context->vk_surface, &requirements, &queue_info, physical_devices[i], &device_properties, &device_features,
                                       &context->vk_device.swapchain_info);

        if (result)
        {
            switch (device_properties.deviceType)
            {
                case VK_PHYSICAL_DEVICE_TYPE_OTHER: {
                    DINFO("Selected GPU type is other");
                }
                break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: {
                    DINFO("Seleced GPU type is intergrated");
                }
                break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: {
                    DINFO("Seleced GPU type is discrete");
                }
                break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: {
                    DINFO("Seleced GPU type is virtual");
                }
                break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU: {
                    DINFO("Seleced GPU type is CPU");
                }
                break;
                default: {
                    DINFO("%s GPU doesnt meet the requirements", device_properties.deviceName);
                    return false;
                }
                break;
            }
            DINFO("GPU Driver version: %d.%d.%d", VK_VERSION_MAJOR(device_properties.driverVersion),
                  VK_VERSION_MINOR(device_properties.driverVersion), VK_VERSION_PATCH(device_properties.driverVersion));

            // Vulkan API version.
            DINFO("Vulkan API version: %d.%d.%d", VK_VERSION_MAJOR(device_properties.apiVersion), VK_VERSION_MINOR(device_properties.apiVersion),
                  VK_VERSION_PATCH(device_properties.apiVersion));

            // Memory information
            for (u32 j = 0; j < device_memory.memoryHeapCount; ++j)
            {
                f32 memorySizeGib = (((f32)device_memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
                if (device_memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                {
                    DINFO("Local GPU memory: %.2f GiB", memorySizeGib);
                }
                else
                {
                    DINFO("Shared System memory: %.2f GiB", memorySizeGib);
                }
            }
            context->vk_device.physical   = physical_devices[i];
            context->vk_device.properties = device_properties;
            context->vk_device.features   = device_features;
            context->vk_device.memory     = device_memory;

            context->vk_device.compute_queue_index  = queue_info.compute_family_index;
            context->vk_device.graphics_queue_index = queue_info.graphics_family_index;
            context->vk_device.transfer_queue_index = queue_info.transfer_family_index;
            context->vk_device.present_queue_index  = queue_info.present_family_index;

            break;
        }
    }
    if (!context->vk_device.physical)
    {
        DERROR("GPU's dont meet the minimum requirements!!");
        return false;
    }

    DINFO("Physical device selected");
    return true;
}
static b8 is_device_suitable(const VkSurfaceKHR surface, const vulkan_physical_device_requirements *requirements,
                             vulkan_physical_device_queue_family_info *queue_info, VkPhysicalDevice device,
                             const VkPhysicalDeviceProperties *properties, const VkPhysicalDeviceFeatures *features,
                             vulkan_swapchain_support_info *out_swapchain_info)
{
    queue_info->transfer_family_index = -1;
    queue_info->graphics_family_index = -1;
    queue_info->present_family_index  = -1;
    queue_info->compute_family_index  = -1;
    queue_info->compute_family_index  = -1;

    if (requirements->discrete_gpu)
    {
        if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            DINFO("Discrete gpu required but not found. Skipping");
            return false;
        }
    }
    // find queues

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0);

    VkQueueFamilyProperties *queue_family_properties = darray_reserve(VkQueueFamilyProperties, queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_family_properties);

    DINFO("Graphics | Compute | Transfer | Present | Name");

    u8 minTransferScore = 255;
    for (i32 j = 0; j < queue_family_count; j++)
    {
        u8 currentTransferScore = 0;

        // Graphics queue?
        if (queue_family_properties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queue_info->graphics_family_index = j;
            currentTransferScore++;
        }

        // Compute queue?
        if (queue_family_properties[j].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            queue_info->compute_family_index = j;
            currentTransferScore++;
        }

        // Transfer queue?
        if (queue_family_properties[j].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {

            if (currentTransferScore <= minTransferScore)
            {
                minTransferScore                  = currentTransferScore;
                queue_info->transfer_family_index = j;
            }
        }

        VkBool32 present_support = false;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, j, surface, &present_support));

        if (present_support)
        {
            queue_info->present_family_index = j;
        }
    }

    DINFO("        %d |      %d |       %d |        %d | %s", queue_info->graphics_family_index != -1, queue_info->compute_family_index != -1,
          queue_info->transfer_family_index != -1, queue_info->present_family_index != -1, properties->deviceName);

    if ((!requirements->graphics_queue || (requirements->graphics_queue && queue_info->graphics_family_index != -1)) &&
        (!requirements->compute_queue || (requirements->compute_queue && queue_info->compute_family_index != -1)) &&
        (!requirements->present_queue || (requirements->present_queue && queue_info->present_family_index != -1)) &&
        (!requirements->transfer_queue || (requirements->transfer_queue && queue_info->transfer_family_index != -1)))
    {

        DTRACE("Graphics Family Index: %i", queue_info->graphics_family_index);
        DTRACE("Transfer Family Index: %i", queue_info->transfer_family_index);
        DTRACE("Compute Family Index:  %i", queue_info->compute_family_index);
        DTRACE("Present Family Index:  %i", queue_info->present_family_index);

        // check for swapchain

        //  see if device supports swapchain extension
        u32 extension_count = 0;
        VK_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &extension_count, 0));

        VkExtensionProperties *extension_properties = dallocate(sizeof(VkExtensionProperties) * extension_count, MEMORY_TAG_RENDERER);

        VK_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &extension_count, extension_properties));

        u32 required_extensions = darray_length(requirements->required_device_extensions);

        DINFO("Checking required vulkan extensions support...");

        for (i32 i = 0; i < required_extensions; i++)
        {
            b8 found = false;
            for (i32 j = 0; j < extension_count; j++)
            {
                if (string_compare(requirements->required_device_extensions[i], extension_properties[j].extensionName))
                {
                    found++;
                    break;
                }
            }
            if (!found)
            {
                DERROR("required extension %s is not supported by the GPU! skipping...", requirements->required_device_extensions[i]);
                dfree(extension_properties, sizeof(VkExtensionProperties) * extension_count, MEMORY_TAG_RENDERER);
                return false;
            }
        }

        // query swapchain support
        vk_query_swapchain_support_info(device, surface, out_swapchain_info);

        // we need a minimum of 2 swapchains to

        if (out_swapchain_info->format_count < 1 || out_swapchain_info->present_mode_count < 1)
        {
            if (out_swapchain_info->formats)
            {
                dfree(out_swapchain_info->formats, sizeof(VkSurfaceFormatKHR) * out_swapchain_info->format_count, MEMORY_TAG_RENDERER);
            }
            if (out_swapchain_info->present_modes)
            {
                dfree(out_swapchain_info->present_modes, sizeof(VkPresentModeKHR) * out_swapchain_info->present_mode_count, MEMORY_TAG_RENDERER);
            }
            DINFO("Curent GPU doesnt meet the swapchain requirements, skipping...");
            return false;
        }

        return true;
    }
    return false;
}

static void write_str(char *buf, char *str, i32 *start)
{
    i32 str_len = string_length(str);
    for (i32 i = 0; i < str_len; i++)
    {
        buf[*start + i] = str[i];
    }
    buf[*start + str_len]     = ' ';
    buf[*start + str_len + 1] = '|';
    buf[*start + str_len + 2] = ' ';
    *start += str_len + 3;
}

static void vk_query_swapchain_support_info(VkPhysicalDevice device, VkSurfaceKHR surface, vulkan_swapchain_support_info *out_swapchain_info)
{
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &out_swapchain_info->surface_capabilities));

    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &out_swapchain_info->format_count, 0));

    if (out_swapchain_info->format_count != 0)
    {
        out_swapchain_info->formats = dallocate(sizeof(VkSurfaceFormatKHR) * out_swapchain_info->format_count, MEMORY_TAG_RENDERER);
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &out_swapchain_info->format_count, out_swapchain_info->formats));
    }

    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &out_swapchain_info->present_mode_count, 0));

    if (out_swapchain_info->present_mode_count != 0)
    {
        out_swapchain_info->present_modes = dallocate(sizeof(VkPresentModeKHR) * out_swapchain_info->present_mode_count, MEMORY_TAG_RENDERER);
        VK_CHECK(
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &out_swapchain_info->present_mode_count, out_swapchain_info->present_modes));
    }
}

static void populate_buf(char *buf, VkQueueFlags queueFlags)
{

    i32 ind = 0;

    if (queueFlags & VK_QUEUE_GRAPHICS_BIT)
    {
        write_str(buf, "VK_QUEUE_GRAPHICS_BIT", &ind);
    }
    if (queueFlags & VK_QUEUE_COMPUTE_BIT)
    {
        write_str(buf, "VK_QUEUE_COMPUTE_BIT", &ind);
    }
    if (queueFlags & VK_QUEUE_TRANSFER_BIT)
    {
        write_str(buf, "VK_QUEUE_TRANSFER_BIT", &ind);
    }
    if (queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
    {
        write_str(buf, "VK_QUEUE_SPARSE_BINDING_BIT", &ind);
    }
    if (queueFlags & VK_QUEUE_PROTECTED_BIT)
    {
        write_str(buf, "VK_QUEUE_PROTECTED_BIT", &ind);
    }
    if (queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
    {
        write_str(buf, "VK_QUEUE_VIDEO_DECODE_BIT_KHR", &ind);
    }
    if (queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
    {
        write_str(buf, "VK_QUEUE_VIDEO_ENCODE_BIT_KHR", &ind);
    }
    if (queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV)
    {
        write_str(buf, "VK_QUEUE_OPTICAL_FLOW_BIT_NV", &ind);
    }
    buf[ind] = '\0';
}

b8 vk_destroy_device(vulkan_context *context)
{
    vkDestroyDevice(context->vk_device.logical, 0);
    return true;
}
