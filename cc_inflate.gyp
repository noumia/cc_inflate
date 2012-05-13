# This file is used with the GYP meta build system.
# http://code.google.com/p/gyp/

#
# python.exe .\gyp\gyp -f msvs -G msvs_version=2010 "--depth=%CD%" %~n0.gyp
#

{
  "target_defaults": {
    "default_configuration": "Release",
    "configurations": {
      "Debug": {
        "defines": [ "DEBUG", "_DEBUG", "_CRT_SECURE_NO_WARNINGS" ],
        "msvs_settings": {
          "VCCLCompilerTool": {
            "RuntimeLibrary": 1, # static debug
            "WarningLevel"  : 3,
            "Optimization"  : 0,
          },
        },
      },
      "Release": {
        "defines": [ "NDEBUG", "_CRT_SECURE_NO_WARNINGS" ],
        "msvs_settings": {
          "VCCLCompilerTool": {
            "RuntimeLibrary": 0, # static release
            "WarningLevel"  : 3,
          },
        },
      }
    },
    "msvs_settings": {
      "VCCLCompilerTool": {
      },
      "VCLibrarianTool": {
      },
      "VCLinkerTool": {
        "GenerateDebugInformation": "true",
      },
    },
    "conditions": [
      ['OS == "win"', {
        "defines": [
          "WIN32"
        ],
      }]
    ],
  },


  "targets": [
    {
      "target_name": "cc_inflate",
      "type": "static_library",
      "include_dirs": [ ".", "./include" ],
      "direct_dependent_settings": {
        "include_dirs": [ "./include" ],
      },
      "sources": [ "./src/cc_inflate.c", "./src/cc_gzip.c", "./src/cc_zlib.c" ]
    },

    {
      "target_name": "cc_ungzip",
      "type": "executable",
      "dependencies": [ "cc_inflate" ],
      "sources": [ "./test/cc_ungzip.c" ]
    },

  ]
}

