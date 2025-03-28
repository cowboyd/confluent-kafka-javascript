{
  "variables": {
    # may be redefined in command line on configuration stage
    # "BUILD_LIBRDKAFKA%": "<!(echo ${BUILD_LIBRDKAFKA:-1})"
    "BUILD_LIBRDKAFKA%": "<!(node ./util/get-env.js BUILD_LIBRDKAFKA 1)",
    "CKJS_LINKING%": "<!(node ./util/get-env.js CKJS_LINKING static)",
  },
  "targets": [
    {
      "target_name": "confluent-kafka-javascript",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "xcode_settings": { "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "CLANG_CXX_LIBRARY": "libc++",
        "MACOSX_DEPLOYMENT_TARGET": "10.7",
      },
      "msvs_settings": {
        "VCCLCompilerTool": { "ExceptionHandling": 1 },
      },
      'sources': [
        'src/binding.cc',
        'src/callbacks.cc',
        'src/common.cc',
        'src/config.cc',
        'src/connection.cc',
        'src/errors.cc',
        'src/kafka-consumer.cc',
        'src/producer.cc',
        'src/topic.cc',
        'src/workers.cc',
        'src/admin.cc'
      ],
      "include_dirs": [
        "<!(node -p \"require('node-addon-api').include_dir\")",
        "<(module_root_dir)/"
      ],
      'conditions': [
        [
          'OS=="win"',
          {
            'actions': [
              {
                'action_name': 'nuget_librdkafka_download',
                'inputs': [
                  'deps/windows-install.py'
                ],
                'outputs': [
                  'deps/precompiled/librdkafka.lib',
                  'deps/precompiled/librdkafkacpp.lib'
                ],
                'message': 'Getting librdkafka from nuget',
                'action': ['python', '<@(_inputs)']
              }
            ],
            'cflags_cc' : [
              '-std=c++20'
            ],
            'msvs_settings': {
              'VCLinkerTool': {
                'AdditionalDependencies': [
                  'librdkafka.lib',
                  'librdkafkacpp.lib'
                ],
                'AdditionalLibraryDirectories': [
                  '../deps/precompiled/'
                ]
              },
              'VCCLCompilerTool': {
                'AdditionalOptions': [
                  '/GR'
                ],
                'AdditionalUsingDirectories': [
                  'deps/precompiled/'
                ],
                'AdditionalIncludeDirectories': [
                  'deps/librdkafka/src',
                  'deps/librdkafka/src-cpp'
                ]
              }
            },
            'include_dirs': [
              '<!(node -p "require(\'node-addon-api\').include_dir")',
              'deps/include'
            ]
          },
          {
            'conditions': [
              [ "<(BUILD_LIBRDKAFKA)==1",
                {
                  "dependencies": [
                    "deps/librdkafka.gyp:librdkafka"
                  ],
                  "include_dirs": [
                    "<!(node -p \"require('node-addon-api').include_dir\")",
                    "deps/librdkafka/src",
                    "deps/librdkafka/src-cpp"
                  ],
                  'conditions': [
                    [
                      'CKJS_LINKING=="dynamic"',
                      {
                        "conditions": [
                            [
                                'OS=="mac"',
                                {
                                  "libraries": [
                                    "../build/deps/librdkafka.dylib",
                                    "../build/deps/librdkafka++.dylib",
                                    "-Wl,-rpath,'$$ORIGIN/../deps'",
                                  ],
                                },
                                {
                                    "libraries": [
                                      "../build/deps/librdkafka.so",
                                      "../build/deps/librdkafka++.so",
                                      "-Wl,-rpath,'$$ORIGIN/../deps'",
                                    ],
                                },
                            ]
                        ]
                      },
                      {
                        "libraries": [
                          "../build/deps/librdkafka-static.a",
                          "../build/deps/librdkafka++.a",
                          "-Wl,-rpath,'$$ORIGIN/../deps'",
                        ],
                      }
                    ],
                  ],
                },
                # Else link against globally installed rdkafka and use
                # globally installed headers.  On Debian, you should
                # install the librdkafka1, librdkafka++1, and librdkafka-dev
                # .deb packages.
                {
                  "libraries": ["-lrdkafka", "-lrdkafka++"],
                  "include_dirs": [
                    "<!(node -p \"require('node-addon-api').include_dir\")",
                    "/usr/include/librdkafka",
                    "/usr/local/include/librdkafka",
                    "/opt/include/librdkafka",
                  ],
                },
              ],
              [
                'OS=="linux"',
                {
                  'cflags_cc' : [
                    '-std=c++20'
                  ],
                  'cflags_cc!': [
                    '-fno-rtti'
                  ]
                }
              ],
              [
                'OS=="mac"',
                {
                  'xcode_settings': {
                    'MACOSX_DEPLOYMENT_TARGET': '10.11',
                    'GCC_ENABLE_CPP_RTTI': 'YES',
                    'OTHER_LDFLAGS': [
                      '-L/usr/local/opt/openssl/lib'
                    ],
                    'OTHER_CPLUSPLUSFLAGS': [
                      '-I/usr/local/opt/openssl/include',
                      '-std=c++20'
                    ],
                  },
                }
              ]
            ]
          }
        ]
      ]
    }
  ]
}
