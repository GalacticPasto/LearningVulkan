#include "vulkan_device.h"
#include "containers/darray.h"
#include "core/dstring.h"
#include "core/logger.h"

typedef struct vulkan_physical_device_requirements
{
    b8 graphics_queue_index;
    b8 compute_queue_index;
    b8 transfer_queue_index;
    b8 discrete_gpu;

} vulkan_physical_device_requirements;

typedef struct vulkan_physical_device_queue_family_info
{
    u32 graphics_family_index;
    u32 present_family_index;
    u32 compute_family_index;
    u32 transfer_family_index;
} vulkan_physical_device_queue_family_info;

static b8   select_physical_device(vulkan_context *context);
static b8   is_device_suitable(const vulkan_physical_device_requirements requirements,
                               vulkan_physical_device_queue_family_info *queue_info, VkPhysicalDevice device,
                               const VkPhysicalDeviceProperties *properties, const VkPhysicalDeviceFeatures *features);
static void populate_buf(char *queue_types_buf, VkQueueFlags queueFlags);

b8 vk_create_device(vulkan_context *context)
{
    if (!select_physical_device(context))
    {
        return false;
    }

    return true;
}

b8 vk_destroy_device(vulkan_context *context)
{
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

    for (i32 i = 0; i < device_count; i++)
    {

        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(physical_devices[i], &device_properties);

        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceFeatures(physical_devices[i], &device_features);

        VkPhysicalDeviceMemoryProperties device_memory;
        vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &device_memory);

        vulkan_physical_device_requirements requirements = {};

        // for now we only have these requirements
        requirements.discrete_gpu         = true;
        requirements.graphics_queue_index = true;
        requirements.compute_queue_index  = true;
        requirements.transfer_queue_index = true;

        vulkan_physical_device_queue_family_info queue_info = {};
        b8                                       result =
            is_device_suitable(requirements, &queue_info, physical_devices[i], &device_properties, &device_features);

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
            DINFO("Vulkan API version: %d.%d.%d", VK_VERSION_MAJOR(device_properties.apiVersion),
                  VK_VERSION_MINOR(device_properties.apiVersion), VK_VERSION_PATCH(device_properties.apiVersion));

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
            context->device.physical_device = physical_devices[i];
            context->device.properties      = device_properties;
            context->device.features        = device_features;
            context->device.memory          = device_memory;

            context->device.compute_queue_index  = queue_info.compute_family_index;
            context->device.graphics_queue_index = queue_info.graphics_family_index;
            context->device.transfer_queue_index = queue_info.transfer_family_index;
        }
    }
    return false;
}
static b8 is_device_suitable(const vulkan_physical_device_requirements requirements,
                             vulkan_physical_device_queue_family_info *queue_info, VkPhysicalDevice device,
                             const VkPhysicalDeviceProperties *properties, const VkPhysicalDeviceFeatures *features)
{
    queue_info->transfer_family_index = -1;
    queue_info->graphics_family_index = -1;
    queue_info->present_family_index  = -1;
    queue_info->compute_family_index  = -1;

    if (requirements.discrete_gpu)
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

    DINFO("Graphics | Compute | Transfer | Name");

    u8 minTransferScore = 255;
    for (i32 j = 0; j < queue_family_count; j++)
    {
#if 0
            char queue_types_buf[1024];
            dset_memory(queue_types_buf, ' ', 1024);
            populate_buf(queue_types_buf, queue_family_properties[j].queueFlags);
            DINFO("Queue Count: %d , Queue Flags: %s", queue_family_properties[j].queueCount, queue_types_buf);
#endif
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
    }

    DINFO("       %d |       %d |        %d | %s", queue_info->graphics_family_index != -1,
          queue_info->compute_family_index != -1, queue_info->transfer_family_index != -1, properties->deviceName);

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
