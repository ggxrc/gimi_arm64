# Phase 4 Research: Pipeline de Texturas & Compatibilidade de Formatos Móveis

## Domain Knowledge
- **Vulkan Textures (Images):** In Vulkan, textures are `VkImage` objects bound to `VkDeviceMemory`. Textures are accessed in shaders via `VkDescriptorSet` containing `VkImageView` and `VkSampler`.
- **Interception Points:**
  - `vkCreateImage` / `vkBindImageMemory` (to track creation and hash base data if possible).
  - `vkUpdateDescriptorSets` (to intercept when the game binds a texture to a descriptor set, so we can swap the `VkImageView` if the hash matches a rule).
  - `vkCmdCopyBufferToImage` (where the actual pixel data upload happens; this is where we can hash the original texture data to identify it).
- **Mobile Texture Formats:**
  - Standard PC textures (BC7/DXT) are not universally supported on mobile (ARM Mali/Adreno).
  - Standard mobile compressed formats are **ASTC** (Adaptive Scalable Texture Compression) and **ETC2**.
  - `gimi_arm64` needs to load these mobile formats (or raw RGBA8 if uncompressed) from mod files.
- **Texture Replacement Logic:**
  1. Intercept `vkCmdCopyBufferToImage`. Calculate the xxHash64 of the buffer data being uploaded. This is the texture hash.
  2. Map the `VkImage` handle to this hash in the `HashRegistry`.
  3. Intercept `vkCreateImageView`. Map the `VkImageView` handle to the `VkImage`'s hash.
  4. Intercept `vkUpdateDescriptorSets`. If a descriptor contains a `VkImageView` with a known hash, and that hash is in the `ModDatabase` (`[TextureOverride]`), replace the `imageView` in the descriptor write with the modded texture's `VkImageView`.
- **Mod Texture Loading:**
  - Need a basic loader for ASTC/ETC2/RGBA8 `.ktx` or raw binary files. We will assume a simple binary format or KTX for V1. Since we just need the raw bytes to push into a new `VkImage`, we can load the raw bytes from disk/memory and create a VRAM buffer, then do a GPU copy to the modded `VkImage`.

## Actionable Strategy
- **TEX-01:** Implement intercepts for `vkCmdCopyBufferToImage` to hash textures, and `vkUpdateDescriptorSets` to swap them.
- **TEX-02:** Create a `TextureLoader` that can read mobile texture formats and upload them to a new Vulkan `VkImage`.
- **TEX-03:** Wire the `HashRegistry`, `TextureLoader`, and `ModDatabase` together in `vkUpdateDescriptorSets`.
