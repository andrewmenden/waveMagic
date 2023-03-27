#pragma once

#include <iostream>
#include <fstream>
#include <cmath>


namespace wave
{
    const double PI = acos(-1);
    typedef char byte;
    using std::ofstream;
    using std::ifstream;
    using std::string;
    using std::cout;

    void reinterpretFile(string inData, string outData, bool printProperties = false)
    {
        ifstream inFile(inData, ifstream::binary);
        ofstream outFile(outData, std::ios::binary);

        short bytesPerSample; //in bytes
        int dataSize; //in bytes

        if (printProperties = true)
        {
            char bytes4[4];
            char bytes2[2];

            cout << inData << '\n';
            inFile.read(bytes4, 4);
            cout << bytes4[0] << bytes4[1] << bytes4[2] << bytes4[3] << '\n';
            inFile.read(bytes4, 4);
            cout << "file size: " << *(int*)&bytes4 << " bytes\n";
            inFile.read(bytes4, 4);
            cout << bytes4[0] << bytes4[1] << bytes4[2] << bytes4[3] << '\n';
            inFile.read(bytes4, 4);
            cout << bytes4[0] << bytes4[1] << bytes4[2] << bytes4[3] << '\n';
            inFile.read(bytes4, 4);
            cout << "subchunk: " << *(int*)&bytes4 << '\n';
            inFile.read(bytes2, 2);
            cout << "format (PCM?): " << *(short*)&bytes2 << '\n';
            inFile.read(bytes2, 2);
            cout << "channels: " << *(short*)&bytes2 << '\n';
            inFile.read(bytes4, 4);
            cout << "sample rate: " << *(int*)&bytes4 << '\n';
            inFile.read(bytes4, 4);
            cout << "byte rate: " << *(int*)&bytes4 << '\n';
            inFile.read(bytes2, 2);
            cout << "block align: " << *(short*)&bytes2 << '\n';
            inFile.read(bytes2, 2);
            bytesPerSample = (*(short*)&bytes2) >> 3;
            cout << "bits per sample: " << *(short*)&bytes2 << " bits\n";
            inFile.read(bytes4, 4);
            cout << bytes4[0] << bytes4[1] << bytes4[2] << bytes4[3] << '\n';
            inFile.read(bytes4, 4);
            dataSize = *(int*)&bytes4;
            cout << "data size: " << dataSize << " bytes\n";
        }
        else
        {
            char bytes4[4];
            char bytes2[2];

            inFile.read(bytes4, 4);
            inFile.read(bytes4, 4);
            inFile.read(bytes4, 4);
            inFile.read(bytes4, 4);
            inFile.read(bytes4, 4);
            inFile.read(bytes2, 2);
            inFile.read(bytes2, 2);
            inFile.read(bytes4, 4);
            inFile.read(bytes4, 4);
            inFile.read(bytes2, 2);
            inFile.read(bytes2, 2);
            bytesPerSample = (*(short*)&bytes2) >> 3;
            inFile.read(bytes4, 4);
            inFile.read(bytes4, 4);
            dataSize = *(int*)&bytes4;
        }

        for (int i = 0; i < dataSize / bytesPerSample; i++)
        {
            int data = 0;
            char* raw = new char[bytesPerSample];

            inFile.read(raw, bytesPerSample);
            for (int i = 0; i < bytesPerSample; i++)
            {
                data += raw[i] << (i << 3);
            }

            outFile << data << ',';

            delete[] raw;
        }

        inFile.close();
        outFile.close();
    }

    short getBytesPerSample(string waveFile)
    {
        ifstream file(waveFile);
        char byte2[2];
        file.seekg(34, std::ios_base::beg);
        file.read(byte2, 2);
        file.close();
        return (*(short*)&byte2) >> 3;
    }
    short getBytesPerSample(ifstream& file)
    {
        file.seekg(34, std::ios_base::beg);
        char byte2[2];
        file.read(byte2, 2);
        return (*(short*)&byte2) >> 3;
    }
    int getDataSize(string waveFile)
    {
        ifstream file(waveFile);
        file.seekg(40, std::ios_base::beg);
        char byte4[4];
        file.read(byte4, 4);
        file.close();
        return *(int*)&byte4;
    }
    int getDataSize(ifstream& file)
    {
        file.seekg(40, std::ios_base::beg);
        char byte4[4];
        file.read(byte4, 4);
        return *(int*)&byte4;
    }

    int byteArrayAsInt(char bytes[], int size)
    {
        int value = 0;
        for (int i = 0; i < size; i++)
        {
            value += bytes[i] << (i << 3);
        }
        return value;
    }

    //with linear interpolation
    short waveFileAsFunction(unsigned short x, ifstream& file)
    {
        int dataSize = getDataSize(file);
        short bytesPerSample = getBytesPerSample(file);

        int totalDataPoints = dataSize / bytesPerSample;

        float offset = x * (totalDataPoints / (float)65536);
        int y0;
        int y1;
        char* bytes = new char[bytesPerSample];

        file.seekg(44 + (int)offset * bytesPerSample, std::ios_base::beg);
        file.read(bytes, bytesPerSample);
        y0 = byteArrayAsInt(bytes, bytesPerSample);

        if ((int)offset == totalDataPoints - 1) //need to loop around
        {
            file.seekg(44, std::ios_base::beg);
        }

        file.read(bytes, bytesPerSample);
        y1 = byteArrayAsInt(bytes, bytesPerSample);

        delete[] bytes;

        return (short)(y0 + (offset - (int)offset) * (y1 - y0) + 0.5);
    }
    //with linear interpolation
    short waveFileAsFunction(unsigned short x, string waveFile)
    {
        ifstream file(waveFile, ifstream::binary);
        short value = waveFileAsFunction(x, file);
        file.close();
        return value;
    }

    class wavestream
    {
    private:
        int fileSize;
        int subChunk;
        short audioFormat; //always pcm
        short channels;
        int sampleRate;
        int byteRate;
        short blockAlign;
        short bitsPerSample;
        int dataSize;

        ofstream tempFile;
        int writes;
    public:
        wavestream(short channels = 1, int sampleRate = 44100, short bitsPerSample = 16)
        {
            this->fileSize = 44 + 8; //size of header data + 8 (because it's weird)
            this->dataSize = 0;
            this->writes = 0;
            this->audioFormat = 1;
            this->subChunk = 16;

            this->channels = channels;
            this->sampleRate = sampleRate;
            this->bitsPerSample = bitsPerSample;
            this->byteRate = (sampleRate * bitsPerSample * channels) >> 3; //rounding anyways
            this->blockAlign = (bitsPerSample * channels) >> 3; //again
            tempFile.open("tempdata.wavdata", std::ios::binary);
        }
        ~wavestream()
        {
            tempFile.close();
        }
        void operator<<(short a)
        {
            tempFile.write((char*)&a, sizeof(a));
            dataSize += 8 * sizeof(a);
            fileSize += 8 * sizeof(a);
            writes += 1;
        }
        void saveAs(string name)
        {
            ofstream file(name, std::ios::binary);

            file << "RIFF";
            file.write((char*)(&fileSize), sizeof(fileSize));
            file << "WAVE";
            file << "fmt";
            file << (char)0x20;
            file.write((char*)(&subChunk), sizeof(subChunk));
            file.write((char*)(&audioFormat), sizeof(audioFormat));
            file.write((char*)(&channels), sizeof(channels));
            file.write((char*)(&sampleRate), sizeof(sampleRate));
            file.write((char*)(&byteRate), sizeof(byteRate));
            file.write((char*)(&blockAlign), sizeof(blockAlign));
            file.write((char*)(&bitsPerSample), sizeof(bitsPerSample));
            file << "data";
            file.write((char*)(&dataSize), sizeof(dataSize));

            tempFile.close();
            ifstream tempFileIn("tempdata.wavdata", ifstream::binary);
            for (int i = 0; i < writes; i++)
            {
                char data[2];
                tempFileIn.read(data, sizeof(data));
                file.write(data, sizeof(data));
            }

            file.close();
            remove("tempdata.wavdata");
        }
        void frequencyWithWav(float hz, unsigned short volume, float duration, string waveFile)
        {
            int samples = duration * sampleRate;
            ifstream file(waveFile, ifstream::binary);
            int fileSamples = getDataSize(file) / getBytesPerSample(file);
            float volumeProportion = volume / (float)0xffff;
            for (int i = 0; i < samples; i++)
            {
                int offset = i * 0xffff/(sampleRate / (double)hz);
                while (offset > 0xffff)
                {
                    offset -= 0xffff;
                }
                short amplitude = waveFileAsFunction((short)offset, file);
                operator<<((volumeProportion)*waveFileAsFunction(offset, file));
            }
            file.close();
        }
    };

}
