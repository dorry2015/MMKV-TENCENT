/*
 * Tencent is pleased to support the open source community by making
 * MMKV available.
 *
 * Copyright (C) 2018 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *       https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "pch.h"

#include <MMKV/MMKV.h>
#include <MMKV/MMBuffer.h>
#include <chrono>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <io.h>
#include<fstream>

using namespace std;
using namespace mmkv;

ofstream out2file;

std::string string_To_UTF8(const std::string &str) {
    int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

    wchar_t *pwBuf = new wchar_t[nwLen + 1]; //一定要加1，不然会出现尾巴
    ZeroMemory(pwBuf, nwLen * 2 + 2);

    ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

    int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

    char *pBuf = new char[nLen + 1];
    ZeroMemory(pBuf, nLen + 1);

    ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

    std::string retStr(pBuf);

    delete[] pwBuf;
    delete[] pBuf;

    pwBuf = NULL;
    pBuf = NULL;

    return retStr;
}

void printmmkv(string rootDir,string mapid) {
    //wstring rootDir = L"E:\\Work\\Code\\MMKV\\Win32\\Debug";
    std::wstring wsTmp(rootDir.begin(), rootDir.end());
    MMKV::initializeMMKV(wsTmp, MMKVLogNone);
    MMKV *mmkv = MMKV::mmkvWithID(mapid);
    vector<string> arrStringKeys = mmkv->allKeys();
    out2file << string_To_UTF8("【") << mapid << string_To_UTF8 ("】") << endl;
    if (mapid == "xweb_debug") {
        cout << "debug" << endl;
    }
    for (size_t i = 0; i < arrStringKeys.size(); i++) {
        string one_record = "";
        string key = arrStringKeys[i];
        string string_result = "E";
        int32_t int32_result = 0;
        uint32_t uint32_result;
        int64_t int64_result;
        uint64_t uint64_result;
        bool bool_result = false;
        double double_result = 0;
        float float_result = 0;
        vector<string> vector_result;
        size_t value_size_actual = mmkv->getValueSize(key, true);
        size_t value_size = mmkv->getValueSize(key, false);
        bool isOK = mmkv->getVector(key, vector_result);
        one_record.append(key);
        one_record.append("=");
        isOK = mmkv->getString(key, string_result);
        if (vector_result.size() > 0 && string_result.at(0) != 0) {
            for (size_t j = 0; j < vector_result.size(); j++) {
                one_record.append(vector_result.at(j) + " ");
            }
        } else {
            string_result = "E";
            isOK = mmkv->getString(key, string_result);
            if (string_result == "E" || string_result.length() == 0 || string_result.at(0) == 0) {
                int32_result = mmkv->getInt32(key);
                one_record.append(" int32(" + std::to_string(int32_result) + ")");
                uint32_result = mmkv->getUInt32(key);
                one_record.append(" uint32(" + std::to_string(uint32_result) + ")");
                int64_result = mmkv->getInt64(key);
                one_record.append(" int64(" + std::to_string(int64_result) + ")");
                uint64_result = mmkv->getUInt64(key);
                one_record.append(" uint64(" + std::to_string(uint64_result) + ")");
                float_result = mmkv->getFloat(key);
                one_record.append(" float(" + std::to_string(float_result) + ")");
                double_result = mmkv->getDouble(key);
                one_record.append(" double(" + std::to_string(double_result) + ")");
                bool_result = mmkv->getBool(key);
                one_record.append(" bool(" + std::to_string(bool_result) + ")");
                MMBuffer mmbuf = mmkv->getBytes(key);
            } else {
                one_record.append(string_result);
            }
        }

        out2file << one_record.c_str() << endl;
    }
    out2file << endl << endl;
}

vector<string> getAllFileNames(const string &folder_path) {
    _finddata_t file;
    long flag;
    vector<string> vector_result;
    string filename = folder_path + "\\*.crc";
    if ((flag = _findfirst(filename.c_str(), &file)) == -1){
        cout << "There is no such type file" << endl;
    } else {
        //通过前面的_findfirst找到第一个文件
        string name = folder_path + "\\" + file.name;
        int index = name.find_last_of(".");
        string realname = name.substr(0, index);
        vector_result.push_back(realname);
        //依次寻找以后的文件
        while (_findnext(flag, &file) == 0) {
            string name = string(folder_path + "\\" + string(file.name));
            int index = name.find_last_of(".");
            string realname = name.substr(0, index);
            vector_result.push_back(realname);
        }
    }
    _findclose(flag);
    return vector_result;
}


int main(int argc,char* argv[]) {
    locale::global(locale(""));
    wcout.imbue(locale(""));
    if (argc < 2) {
        cout << "输入文件夹或者文件名";
        return 0;
    }

    out2file.open("result.txt", ios::out);
    unsigned char smarker[3];
    smarker[0] = 0xEF;
    smarker[1] = 0xBB;
    smarker[2] = 0xBF;

    out2file << smarker;
    struct stat s;
    string path(argv[1]);
    if (stat(path.c_str(), &s) == 0) {
        if (s.st_mode & S_IFDIR) {
            vector<string> result = getAllFileNames(path);
            for (size_t i = 0; i < result.size(); i++) {
                string itemname = result.at(i);
                int index = itemname.find_last_of("\\");
                std::string folderPath = itemname.substr(0, index);
                std::string filename = itemname.substr(index + 1, -1);
                printmmkv(folderPath, filename);
            }
                
        } else if (s.st_mode & S_IFREG) {
            int index = path.find_last_of("\\");
            std::string folderPath = path.substr(0, index);
            std::string filename = path.substr(index + 1, -1);
            printmmkv(folderPath, filename);
        } else {
            cout << "not file not directory" << endl;
        }
    } else {
        cout << "error, doesn't exist" << endl;
    }
    out2file.close();
}
