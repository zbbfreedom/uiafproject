# Microsoft Developer Studio Generated NMAKE File, Based on nhdemo.dsp
!IF "$(CFG)" == ""
CFG=nhdemo - Win32 Debug
!MESSAGE No configuration specified. Defaulting to nhdemo - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "nhdemo - Win32 Release" && "$(CFG)" != "nhdemo - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nhdemo.mak" CFG="nhdemo - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nhdemo - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "nhdemo - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nhdemo - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\nhdemo.exe"


CLEAN :
	-@erase "$(INTDIR)\getwords.obj"
	-@erase "$(INTDIR)\nhdemo.obj"
	-@erase "$(INTDIR)\nhdemo.pch"
	-@erase "$(INTDIR)\nhdemo.res"
	-@erase "$(INTDIR)\nhdemoDlg.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\nhdemo.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\nhdemo.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x804 /fo"$(INTDIR)\nhdemo.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\nhdemo.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\nhdemo.pdb" /machine:I386 /out:"$(OUTDIR)\nhdemo.exe" 
LINK32_OBJS= \
	"$(INTDIR)\getwords.obj" \
	"$(INTDIR)\nhdemo.obj" \
	"$(INTDIR)\nhdemoDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\nhdemo.res"

"$(OUTDIR)\nhdemo.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "nhdemo - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\nhdemo.exe"


CLEAN :
	-@erase "$(INTDIR)\getwords.obj"
	-@erase "$(INTDIR)\nhdemo.obj"
	-@erase "$(INTDIR)\nhdemo.pch"
	-@erase "$(INTDIR)\nhdemo.res"
	-@erase "$(INTDIR)\nhdemoDlg.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\nhdemo.exe"
	-@erase "$(OUTDIR)\nhdemo.ilk"
	-@erase "$(OUTDIR)\nhdemo.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\nhdemo.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x804 /fo"$(INTDIR)\nhdemo.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\nhdemo.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\nhdemo.pdb" /debug /machine:I386 /out:"$(OUTDIR)\nhdemo.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\getwords.obj" \
	"$(INTDIR)\nhdemo.obj" \
	"$(INTDIR)\nhdemoDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\nhdemo.res"

"$(OUTDIR)\nhdemo.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("nhdemo.dep")
!INCLUDE "nhdemo.dep"
!ELSE 
!MESSAGE Warning: cannot find "nhdemo.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "nhdemo - Win32 Release" || "$(CFG)" == "nhdemo - Win32 Debug"
SOURCE=.\getwords.cpp

"$(INTDIR)\getwords.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\nhdemo.pch"


SOURCE=.\nhdemo.cpp

"$(INTDIR)\nhdemo.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\nhdemo.pch"


SOURCE=.\nhdemo.rc

"$(INTDIR)\nhdemo.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\nhdemoDlg.cpp

"$(INTDIR)\nhdemoDlg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\nhdemo.pch"


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "nhdemo - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\nhdemo.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\nhdemo.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "nhdemo - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\nhdemo.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\nhdemo.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

