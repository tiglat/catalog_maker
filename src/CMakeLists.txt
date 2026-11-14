cmake_minimum_required(VERSION 3.15)
project(CatalogMaker VERSION 4.1.3 LANGUAGES CXX)

# ============================================================================
# CMakeLists.txt для CatalogMaker (сгенерировано на основе CatalogMaker.vcxproj)
# ============================================================================

# ---- Стандарт C++ ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# ---- Исходные файлы (.cpp) ----
set(CATALOGMAKER_SRCS
    AnsiStringOperations.cpp
    CatalogMaker.cpp
    GlobalVars.cpp
    SupportFunc.cpp
    UIFunc.cpp
    WideStringOperations.cpp
)

# ---- Заголовочные файлы (.h) (для удобства просмотра в IDE) ----
set(CATALOGMAKER_HDRS
    AnsiStringOperations.h
    CatalogReader.h
    FileList.h
    GlobalVars.h
    IStringOperations.h
    resource.h
    SupportFunc.h
    UIFunc.h
    wcxapi.h
    wcxhead.h
    WideStringOperations.h
)

# ---- Target: библиотека-модуль (плагин WCX для Total Commander) ----
# В оригинале Release|Win32 / Release|x64 собирались как консольные приложения,
# а Debug — как DLL-плагин. Здесь выбран тип MODULE (плагин), так как макросы
# экспорта CATALOGMAKER_EXPORTS / WCX_PLUGIN_EXPORTS указывают на DLL.
add_library(CatalogMaker MODULE
    ${CATALOGMAKER_SRCS}
    ${CATALOGMAKER_HDRS}
    CatalogMaker.rc
)

# ---- Определения препроцессора (из PreprocessorDefinitions в .vcxproj) ----
# Общие для всех конфигураций плагина:
#   WIN32, _WINDOWS, _USRDLL, CATALOGMAKER_EXPORTS, WCX_PLUGIN_EXPORTS,
#   _CRT_SECURE_NO_DEPRECATE, _CRT_NON_CONFORMING_WCSTOK
# Конфигурационные: _DEBUG (Debug) / NDEBUG (Release)
target_compile_definitions(CatalogMaker PRIVATE
    WIN32
    _WINDOWS
    _USRDLL
    CATALOGMAKER_EXPORTS
    WCX_PLUGIN_EXPORTS
    _CRT_SECURE_NO_DEPRECATE
    _CRT_NON_CONFORMING_WCSTOK
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

# Выходное имя и расширение (.wcx для Win32, .wcx64 для x64)
if(WIN32)
    set_target_properties(CatalogMaker PROPERTIES
        OUTPUT_NAME "CatalogMaker"
    )
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set_target_properties(CatalogMaker PROPERTIES SUFFIX ".wcx64")
    else()
        set_target_properties(CatalogMaker PROPERTIES SUFFIX ".wcx")
    endif()
endif()

# Применяем стандарт C++20 к целевой цели
target_compile_features(CatalogMaker PRIVATE cxx_std_20)

# ---- Файл определений модуля (.def) для экспорта функций плагина ----
if(WIN32)
    target_link_options(CatalogMaker PRIVATE "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/CatalogMaker.def")
endif()

# ---- Дополнительные каталоги включений ----
# В исходном .vcxproj AdditionalIncludeDirectories не указывались,
# поэтому в CMake дополнительных include_directories нет.

# ---- Подключаемые библиотеки ----
# В исходном .vcxproj AdditionalDependencies не указывались.

# ============================================================================
# Примечание:
#   В оригинальном проекте конфигурации Release|Win32 и Release|x64
#   собирались как консольные приложения (ConfigurationType = Application),
#   в то время как Debug конфигурации — как динамические библиотеки
#   (ConfigurationType = DynamicLibrary). Данный CMakeLists.txt по умолчанию
#   собирает библиотеку-модуль (плагин). При необходимости собрать
#   консольное приложение, замените MODULE на EXE и укажите
#   set_target_properties(... WIN32_EXECUTABLE FALSE) с подходящими флагами.
# ============================================================================
