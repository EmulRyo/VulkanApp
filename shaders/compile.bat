set glslc=C:/VulkanSDK/1.3.231.1/Bin/glslc.exe
for %%a in (*.vert) do glslc %%a -o %%a.spv
for %%a in (*.frag) do glslc %%a -o %%a.spv
pause