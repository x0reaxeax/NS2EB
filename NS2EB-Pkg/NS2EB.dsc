[Defines]
  PLATFORM_NAME           = NS2EB
  PLATFORM_GUID           = 0405b991-de49-11ef-a5f9-7c8ae1a193a5
  PLATFORM_VERSION        = 1.0
  DSC_SPECIFICATION       = 0x00010005
  OUTPUT_DIRECTORY        = Build/NS2EB
  SUPPORTED_ARCHITECTURES = X64
  BUILD_TARGETS           = DEBUG|RELEASE


[LibraryClasses]
  BaseLib | MdePkg/Library/BaseLib/BaseLib.inf
  UefiLib | MdePkg/Library/UefiLib/UefiLib.inf
  UefiBootServicesTableLib | MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  DebugLib | MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  PrintLib | MdePkg/Library/BasePrintLib/BasePrintLib.inf
  PcdLib | MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  MemoryAllocationLib | MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  BaseMemoryLib | MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  DevicePathLib | MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiRuntimeServicesTableLib | MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  RegisterFilterLib | MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
  UefiApplicationEntryPoint | MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf

[Components]
  NS2EB-Pkg/NS2EB.inf
