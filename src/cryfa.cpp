/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

             =========================================================
             | CRYFA :: A FASTA/FASTQ encryption and decryption tool |
             ---------------------------------------------------------
             |   Diogo Pratas, Morteza Hosseini, Armando J. Pinho    |
             |            {pratas,seyedmorteza,ap}@ua.pt             |
             |       Copyright (C) 2017, University of Aveiro        |
             =========================================================
             
  COMPILE:  g++ -std=c++11 -I cryptopp -o cryfa cryfa.cpp defs.h libcryptopp.a
  
  DEPENDENCIES: https://github.com/weidai11/cryptopp
  sudo apt-get install libcrypto++-dev libcrypto++-doc libcrypto++-utils
  
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#include <iostream>
#include <getopt.h>
#include <chrono>       // time
#include <iomanip>      // setw, setprecision
#include <thread>
#include "def.h"
#include "EnDecrypto.h"
#include "fcn.h"
using std::string;
using std::cout;
using std::cerr;
using std::chrono::high_resolution_clock;
using std::setprecision;
using std::thread;

////////////////////////////////////////////////////////////////////////////////
///////////////////                 M A I N                 ////////////////////
////////////////////////////////////////////////////////////////////////////////
int main (int argc, char* argv[])
{
    // start timer
    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    
    EnDecrypto crpt;
    
    const string inFileName = argv[argc-1];
    static int h_flag, a_flag, v_flag, d_flag;
    string keyFileName;                  // argument of option 'k'
    byte n_threads = DEFAULT_N_THREADS;  // number of threads
    int c;                               // deal with getopt_long()
    int option_index;                    // option index stored by getopt_long()
    opterr = 0;  // force getopt_long() to remain silent when it finds a problem

    static struct option long_options[] =
    {
        {"help",          no_argument, &h_flag, (int) 'h'},   // help
        {"about",         no_argument, &a_flag, (int) 'a'},   // about
        {"verbose",       no_argument, &v_flag, (int) 'v'},   // verbose
        {"decrypt",       no_argument, &d_flag, (int) 'd'},   // decryption mode
        {"key",     required_argument,       0,       'k'},   // key file
        {"thread",  required_argument,       0,       't'},   // #threads >= 1
        {0,                         0,       0,         0}
    };
    
    while (1)
    {
        option_index = 0;
        if ((c = getopt_long(argc, argv, ":havdk:t:",
                             long_options, &option_index)) == -1)    break;

        switch (c)
        {
            case 0:
                // If this option set a flag, do nothing else now.
                if (long_options[option_index].flag != 0)   break;
                cout << "option '" << long_options[option_index].name << "'\n";
                if (optarg)     cout << " with arg " << optarg << '\n';
                break;

            case 'h':   // show usage guide
                h_flag = 1;
                Help();
                break;

            case 'a':   // show about
                a_flag = 1;
                About();
                break;

            case 'v':   // verbose mode
                v_flag = 1;
                break;

            case 'd':   // decompress mode
                d_flag = 1;
                break;
                
            case 'k':   // needs key filename
                keyFileName = (string) optarg;
                break;
            
            case 't':   // number of threads
                n_threads = (byte) stoi((string) optarg);
                break;
                
            default:
                cerr << "Option '" << (char) optopt << "' is invalid.\n";
                break;
        }
    }
    
    if (v_flag) cerr << "Verbose mode on.\n";
    if (d_flag)
    {
        cerr << "Decrypting...\n";
        crpt.decompress(inFileName, keyFileName, v_flag);
        
        // stop timer
        high_resolution_clock::time_point finishTime =
                high_resolution_clock::now();
        // duration in seconds
        std::chrono::duration<double> elapsed = finishTime - startTime;
        cerr << "done in " << std::fixed << setprecision(4) << elapsed.count()
             << " seconds.\n";
        
        return 0;
    }
    
    cerr << "Encrypting...\n";
//    (fileType(inFileName) == 'A')   // compress and encrypt
//            ? crpt.compressFA(inFileName, keyFileName, v_flag)
//            : crpt.compressFQ(inFileName, keyFileName, v_flag);
    
    
    
    //todo. at the moment, multithreading for fastq
    
    
    
    //todo. split file
    std::ifstream inFile(inFileName);
//    splitFile(inFile);
    
    std::ofstream outfile;
    string outName;
    
    string line, hdrRange, qsRange;
    for (byte i = 1; i <= n_threads; ++i)
    {
        outName = "CRYFA_IN" + std::to_string(i-1);
        outfile.open(outName);
        
        for (int j = (i-1) * LINE_BUFFER; j < i * LINE_BUFFER; j += 4)
        {
            if (getline(inFile, line).good())                       // header
            {
                for (const char &c : line)
                    if (hdrRange.find_first_of(c) == string::npos)
                        hdrRange += c;
                outfile << line << '\n';
            }
            if (getline(inFile, line).good())   outfile << line << '\n';
            if (getline(inFile, line).good())   outfile << line << '\n';
            if (getline(inFile, line).good())                       // quality score
            {
                for (const char &c : line)
                    if (qsRange.find_first_of(c) == string::npos)
                        qsRange += c;
                outfile << line << '\n';
            }
        }
        
        outfile.close();    // is a MUST
    }

    inFile.close();
    
    
    thread *arrThread;
    arrThread = new thread[n_threads];
    for (byte t = 0; t < n_threads; ++t)
    {
        arrThread[t] = thread(&EnDecrypto::compressFQ, &crpt,
                              ("CRYFA_IN" + std::to_string(t)),
                              keyFileName,
                              v_flag,
                              hdrRange,
                              qsRange,
                              t
        );
    }
    for (byte t = 0; t < n_threads; ++t)    arrThread[t].join();
    delete[] arrThread;
    
    
    
    
    
    







//    // get size of file
//    infile.seekg(0, infile.end);
//    long size = 1000000;//infile.tellg();
//    infile.seekg(0);
//
//    // allocate memory for file content
//    char *buffer = new char[size];
//
//    // read content of infile
//    infile.read(buffer, size);
//
//    // write to outfile
//    outfile.write(buffer, size);
//
//    // release dynamically-allocated memory
//    delete[] buffer;

//    outfile.close();
//    infile.close();




//    string *strIn = new string[n_threads];
//    string line;
//    string out;
//    std::ifstream in("temp.fq");
//    const bool FASTA = (crpt.fileType(in) == 'A');cerr<<FASTA;
//    while (getline(in, line))
//    {
//        out += line + "\n";
////        out+='\n';
//    }
//    out.pop_back();
//
//    cerr << out;


//    for (byte t = 0; t < n_threads; ++t)
//    {
//        for(unsigned short lineNo = 0; getline(, line))
////    strIn[t]+=;
//    }
    
    
    
    
    
    
    
    
    
    
    // stop timer
    high_resolution_clock::time_point finishTime = high_resolution_clock::now();
    // duration in seconds
    std::chrono::duration<double> elapsed = finishTime - startTime;
    cerr << "done in " << std::fixed << setprecision(4) << elapsed.count()
         << " seconds.\n";

    return 0;
}