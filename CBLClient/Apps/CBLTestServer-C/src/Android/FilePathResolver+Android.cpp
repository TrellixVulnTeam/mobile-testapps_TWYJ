#define LOG_NDEBUG 0
#define LOG_NIDEBUG 0
#define LOG_NDDEBUG 0

#include "../FilePathResolver.h"
#include <android_native_app_glue.h>
#include <android/asset_manager.h>
#include <android/log.h>
#include <unistd.h>
#include <stdexcept>
#include <fstream>
#include <cstdint>
#include <zip.h>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;

extern android_app* GlobalApp;
static string CacheDirectory;

string file_resolution::resolve_path(const string& relativePath, bool unzip) {
    if(CacheDirectory.empty()) {
        CacheDirectory = string(GlobalApp->activity->internalDataPath);
        int last_slash = CacheDirectory.rfind('/');
        CacheDirectory = CacheDirectory.substr(0, last_slash + 1);
        CacheDirectory += "cache/";
    }

    string path = CacheDirectory + "tmp";
    unlink(path.c_str());

    auto* input = AAssetManager_open(GlobalApp->activity->assetManager, relativePath.c_str(), AASSET_MODE_UNKNOWN);
    if(!input) {
        throw domain_error("Unable to open asset");
    }

    ofstream fout(path.c_str(), ios_base::binary);
    fout.exceptions(ios::badbit | ios::failbit);
    char buffer[8192];
    int read;
    while((read = AAsset_read(input, buffer, 8192)) > 0) {
        fout.write(buffer, read);
    }

    fout.flush();
    fout.close();
    AAsset_close(input);

    if(read < 0) {
        throw domain_error("Unable to read asset");
    }

    if(unzip) {
        int e;
        zip_t* zip_input = zip_open(path.c_str(), ZIP_RDONLY, &e);
        if(!zip_input) {
            throw domain_error("Unable to open asset zip");
        }

        zip_int64_t num_entries = zip_get_num_entries(zip_input, ZIP_FL_UNCHANGED);
        for(zip_int64_t i = 0; i < num_entries; i++) {
            zip_file_t* next = zip_fopen_index(zip_input, (zip_uint64_t)i, ZIP_FL_UNCHANGED);
            const char* next_name = zip_get_name(zip_input, (zip_uint64_t)i, ZIP_FL_UNCHANGED);
            int len = strlen(next_name);
            if(next_name[len - 1] == '/') {
                string dir = CacheDirectory + next_name;
                mkdir(dir.c_str(), 0755);
            } else {
                string file = CacheDirectory + next_name;
                ofstream fout(file.c_str(), ios_base::binary|ios_base::ate);
                while((read = zip_fread(next, buffer, 8192)) > 0) {
                    fout.write(buffer, read);
                }

                fout.close();
            }
        }

        zip_close(zip_input);
        string modifiedDir = relativePath;
        int index = modifiedDir.find(".zip");
        if(index == string::npos) {
            throw domain_error("corrupted zip filename");
        }

        modifiedDir.replace(index, 4, "/");
        path = CacheDirectory + modifiedDir;
    }

    return path;
}