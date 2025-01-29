[Defines]
  PLATFORM_NAME           = NS2EB
  PLATFORM_GUID           = 0405b991-de49-11ef-a5f9-7c8ae1a193a5
  PLATFORM_VERSION        = 1.0
  DSC_SPECIFICATION       = 0x00010005
  OUTPUT_DIRECTORY        = Build/NS2EB
  SUPPORTED_ARCHS         = X64
  BUILD_TARGETS           = DEBUG RELEASE

[LibraryClasses]
  BaseLib | MdePkg/Library/BaseLib/BaseLib.inf
  UefiLib | MdePkg/Library/UefiLib/UefiLib.inf
  UefiBootServicesTableLib | MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf

[Components]
  NS2EB-Pkg/NS2EB/NS2EB.inf
