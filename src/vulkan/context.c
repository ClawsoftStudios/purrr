#include "./context.h"

#include <assert.h>

static bool sInitialized = false;
static char *sRequiredExtensions[3] = {0};

static Purrr_Result init();
static Purrr_Result get_instance_extensions(VkExtensionProperties **extensions, uint32_t *count);
static Purrr_Result init_required_extensions();
static Purrr_Result choose_physical_device(VkInstance instance, VkPhysicalDevice *outDevice);
static Purrr_Result query_queue_family(VkPhysicalDevice device, uint32_t *graphicsFamily);
static Purrr_Result create_logical_device(VkPhysicalDevice gpu, uint32_t queueFamily, VkDevice *device);

Purrr_Result _purrr_create_context_vulkan(Purrr_Context_Create_Info createInfo, _Purrr_Context_Vulkan **context) {
  (void)createInfo;

  if (!context) return PURRR_INVALID_ARGS_ERROR;

  _Purrr_Context_Vulkan *ctx = _purrr_malloc_with_header((_Purrr_Object_Header){
    PURRR_VULKAN, _PURRR_OBJECT_CONTEXT, 0
  }, sizeof(*ctx));
  if (!ctx) return PURRR_BUY_MORE_RAM;

  Purrr_Result result = PURRR_SUCCESS;
  if ((result = init()) < PURRR_SUCCESS) return result;

  VkApplicationInfo appInfo = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pNext = VK_NULL_HANDLE,
    .pApplicationName = createInfo.applicationName,
    .applicationVersion = createInfo.applicationVersion,
    .pEngineName = createInfo.engineName,
    .engineVersion = createInfo.engineVersion,
    .apiVersion = VK_API_VERSION_1_1
  };

  VkInstanceCreateInfo instanceCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .pApplicationInfo = &appInfo,
    .ppEnabledExtensionNames = (const char*const*)sRequiredExtensions,
    .enabledExtensionCount = sizeof(sRequiredExtensions)/sizeof(*sRequiredExtensions),
    .ppEnabledLayerNames = (const char *[]){ "VK_LAYER_KHRONOS_validation" },
    .enabledLayerCount = 1
  };

  if (vkCreateInstance(&instanceCreateInfo, VK_NULL_HANDLE, &ctx->instance) != VK_SUCCESS) {
    _purrr_destroy_context_vulkan(ctx);
    return PURRR_INTERNAL_ERROR;
  }

  if ((result = choose_physical_device(ctx->instance, &ctx->gpu)) < PURRR_SUCCESS) {
    _purrr_destroy_context_vulkan(ctx);
    return result;
  }

  if ((result = query_queue_family(ctx->gpu, &ctx->queueFamily)) < PURRR_SUCCESS) {
    _purrr_destroy_context_vulkan(ctx);
    return result;
  }

  if ((result = create_logical_device(ctx->gpu, ctx->queueFamily, &ctx->device)) < PURRR_SUCCESS) {
    _purrr_destroy_context_vulkan(ctx);
    return result;
  }

  vkGetDeviceQueue(ctx->device, ctx->queueFamily, 0, &ctx->queue);

  VkCommandPoolCreateInfo commandPoolCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .queueFamilyIndex = ctx->queueFamily,
  };

  if (vkCreateCommandPool(ctx->device, &commandPoolCreateInfo, VK_NULL_HANDLE, &ctx->commandPool) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  VkDescriptorPoolSize poolSizes[] = {
    (VkDescriptorPoolSize){
      .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 1024
    },
    (VkDescriptorPoolSize){
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1024
    }
  };

  VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
    .maxSets = 2048,
    .poolSizeCount = sizeof(poolSizes)/sizeof(*poolSizes),
    .pPoolSizes = poolSizes,
  };

  if (vkCreateDescriptorPool(ctx->device, &descriptorPoolCreateInfo, VK_NULL_HANDLE, &ctx->descriptorPool) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  {
    VkDescriptorSetLayoutBinding bindings[] = {
      (VkDescriptorSetLayoutBinding){
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_ALL,
        .pImmutableSamplers = VK_NULL_HANDLE
      }
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = VK_NULL_HANDLE,
      .flags = 0,
      .bindingCount = sizeof(bindings)/sizeof(*bindings),
      .pBindings = bindings
    };

    if (vkCreateDescriptorSetLayout(ctx->device, &descriptorSetLayoutCreateInfo, VK_NULL_HANDLE, &ctx->textureLayout) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;
  }

  {
    VkDescriptorSetLayoutBinding bindings[] = {
      (VkDescriptorSetLayoutBinding){
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_ALL,
        .pImmutableSamplers = VK_NULL_HANDLE
      }
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = VK_NULL_HANDLE,
      .flags = 0,
      .bindingCount = sizeof(bindings)/sizeof(*bindings),
      .pBindings = bindings
    };

    if (vkCreateDescriptorSetLayout(ctx->device, &descriptorSetLayoutCreateInfo, VK_NULL_HANDLE, &ctx->uniformBufferLayout) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;
  }

  *context = ctx;

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_destroy_context_vulkan(_Purrr_Context_Vulkan *context) {
  if (!context) return PURRR_INVALID_ARGS_ERROR;

  if (context->descriptorPool) vkDestroyDescriptorPool(context->device, context->descriptorPool, VK_NULL_HANDLE);
  if (context->textureLayout) vkDestroyDescriptorSetLayout(context->device, context->textureLayout, VK_NULL_HANDLE);
  if (context->uniformBufferLayout) vkDestroyDescriptorSetLayout(context->device, context->uniformBufferLayout, VK_NULL_HANDLE);

  if (context->commandPool) vkDestroyCommandPool(context->device, context->commandPool, VK_NULL_HANDLE);
  if (context->device) vkDestroyDevice(context->device, VK_NULL_HANDLE);
  if (context->instance) vkDestroyInstance(context->instance, VK_NULL_HANDLE);

  return PURRR_SUCCESS;
}



Purrr_Result _purrr_context_begin_one_time_command_buffer_vulkan(_Purrr_Context_Vulkan *context, VkCommandBuffer *cmdBuf) {
  if (!context || !cmdBuf) return PURRR_INVALID_ARGS_ERROR;

  VkCommandBufferAllocateInfo allocInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .commandPool = context->commandPool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1,
  };

  if (vkAllocateCommandBuffers(context->device, &allocInfo, cmdBuf) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  VkCommandBufferBeginInfo beginInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    .pInheritanceInfo = VK_NULL_HANDLE
  };

  if (vkBeginCommandBuffer(*cmdBuf, &beginInfo) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  return PURRR_SUCCESS;
}

Purrr_Result _purrr_context_submit_one_time_command_buffer_vulkan(_Purrr_Context_Vulkan *context, VkCommandBuffer cmdBuf) {
  if (!context || !cmdBuf) return PURRR_INVALID_ARGS_ERROR;

  if (vkEndCommandBuffer(cmdBuf) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  VkSubmitInfo submitInfo = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext = VK_NULL_HANDLE,
    .waitSemaphoreCount = 0,
    .pWaitSemaphores = VK_NULL_HANDLE,
    .pWaitDstStageMask = VK_NULL_HANDLE,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmdBuf,
    .signalSemaphoreCount = 0,
    .pSignalSemaphores = VK_NULL_HANDLE
  };

  if (vkQueueSubmit(context->queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;
  if (vkQueueWaitIdle(context->queue) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  vkFreeCommandBuffers(context->device, context->commandPool, 1, &cmdBuf);

  return PURRR_SUCCESS;
}

// definitions:

static Purrr_Result init() {
  if (sInitialized) return PURRR_TRUE;

  Purrr_Result result = PURRR_SUCCESS;
  if ((result = init_required_extensions()) < PURRR_SUCCESS) return result;

  return (sInitialized = true, PURRR_SUCCESS);
}

static Purrr_Result get_instance_extensions(VkExtensionProperties **extensions, uint32_t *count) {
  if (!extensions || !count) return PURRR_INVALID_ARGS_ERROR;

  if (vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, count, VK_NULL_HANDLE)) return PURRR_INTERNAL_ERROR;
  *extensions = malloc(sizeof(**extensions) * *count);
  if (!*extensions) return PURRR_BUY_MORE_RAM;
  if (vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, count, *extensions)) return PURRR_INTERNAL_ERROR;

  return PURRR_SUCCESS;
}

static Purrr_Result init_required_extensions() {
  Purrr_Result result = PURRR_SUCCESS;

  VkExtensionProperties *extensions = NULL;
  uint32_t count = 0;
  if ((result = get_instance_extensions(&extensions, &count)) < PURRR_SUCCESS) return result;

  bool surfaceKHR = false;
  bool surfaceWin32KHR = false;

  for (uint32_t i = 0; i < count; ++i) {
    VkExtensionProperties props = extensions[i];
         if (strcmp(props.extensionName, "VK_KHR_surface") == 0) surfaceKHR = true;
    else if (strcmp(props.extensionName, "VK_KHR_win32_surface") == 0) surfaceWin32KHR = true;
  }

  free(extensions);

  if (!surfaceKHR) return PURRR_INTERNAL_ERROR;

  #if defined(PURRR_PLATFORM_WINDOWS)
    if (!surfaceWin32KHR) return PURRR_INTERNAL_ERROR;
    sRequiredExtensions[0] = "VK_KHR_surface";
    sRequiredExtensions[1] = "VK_KHR_win32_surface";
    sRequiredExtensions[2] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  #else
  #error "Unsupported"
  #endif

  return PURRR_SUCCESS;
}

static Purrr_Result choose_physical_device(VkInstance instance, VkPhysicalDevice *outDevice) {
  if (!instance || !outDevice) return PURRR_INVALID_ARGS_ERROR;

  uint32_t count = 0;
  if (vkEnumeratePhysicalDevices(instance, &count, VK_NULL_HANDLE) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  VkPhysicalDevice *devices = malloc(sizeof(*devices) * count);
  if (!devices) return PURRR_BUY_MORE_RAM;
  if (vkEnumeratePhysicalDevices(instance, &count, devices) != VK_SUCCESS) return PURRR_INTERNAL_ERROR;

  uint32_t bestScore = 0;

  for (uint32_t i = 0; i < count; ++i) {
    uint32_t score = 0;
    VkPhysicalDevice device = devices[i];

    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, VK_NULL_HANDLE, &extensionCount, VK_NULL_HANDLE);

    VkExtensionProperties *extensions = malloc(sizeof(*extensions) * extensionCount);
    if (!extensions) return PURRR_BUY_MORE_RAM;

    vkEnumerateDeviceExtensionProperties(device, VK_NULL_HANDLE, &extensionCount, extensions);

    bool swapchainSupported = false;
    for (uint32_t j = 0; j < extensionCount; ++j) {
      if (strcmp(extensions[j].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
        swapchainSupported = true;
        break;
      }
    }

    free(extensions);

    if (!swapchainSupported) continue;

    VkPhysicalDeviceProperties properties = {0};
    vkGetPhysicalDeviceProperties(device, &properties);

    // VkPhysicalDeviceFeatures features = {0};
    // vkGetPhysicalDeviceFeatures(device, &features);

    switch (properties.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER: score += 0;
      break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: score += 10;
      break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: score += 100;
      break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: score += 25;
      break;
    case VK_PHYSICAL_DEVICE_TYPE_CPU: score += 50;
      break;
    case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
    default: assert(0);
    }

    if (score > bestScore) {
      bestScore = score;
      *outDevice = device;
    }
  }

  free(devices);

  return (bestScore)?PURRR_SUCCESS:PURRR_INTERNAL_ERROR;
}

static Purrr_Result query_queue_family(VkPhysicalDevice device, uint32_t *graphicsFamily) {
  if (!device || !graphicsFamily) return PURRR_INVALID_ARGS_ERROR;

  *graphicsFamily = UINT32_MAX;

  uint32_t count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &count, VK_NULL_HANDLE);

  VkQueueFamilyProperties *queueFamilies = malloc(sizeof(*queueFamilies) * count);
  if (!queueFamilies) return PURRR_BUY_MORE_RAM;

  vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queueFamilies);

  for (uint32_t i = 0; i < count; ++i) {
    VkQueueFamilyProperties properties = queueFamilies[i];

    if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      *graphicsFamily = i;
      free(queueFamilies);
      return PURRR_SUCCESS;
    }
  }

  free(queueFamilies);

  return PURRR_INTERNAL_ERROR;
}

static Purrr_Result create_logical_device(VkPhysicalDevice gpu, uint32_t queueFamily, VkDevice *device) {
  if (!gpu || !device) return PURRR_INVALID_ARGS_ERROR;

  float queuePriority = 0.0f;
  VkDeviceQueueCreateInfo queueCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .queueFamilyIndex = queueFamily,
    .queueCount = 1,
    .pQueuePriorities = &queuePriority
  };

  VkPhysicalDeviceFeatures deviceFeatures = {0};

  const char *extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };

  VkDeviceCreateInfo createInfo = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .queueCreateInfoCount = 1,
    .pQueueCreateInfos = &queueCreateInfo,
    .enabledLayerCount = 0,
    .ppEnabledLayerNames = VK_NULL_HANDLE,
    .enabledExtensionCount = sizeof(extensions)/sizeof(*extensions),
    .ppEnabledExtensionNames = extensions,
    .pEnabledFeatures = &deviceFeatures
  };

  return (vkCreateDevice(gpu, &createInfo, VK_NULL_HANDLE, device) == VK_SUCCESS)?PURRR_SUCCESS:PURRR_INTERNAL_ERROR;
}