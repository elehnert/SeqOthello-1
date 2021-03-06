// This file is a part of SeqOthello. Please refer to LICENSE.TXT for the LICENSE
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <bitset>
#include <chrono>
#include <inttypes.h>
#include <string>
#include <map>
#include <unordered_map>
#include <args.hxx>
#include <io_helper.hpp>
#include <atomic>
#include "socket.h"

using namespace std;


int main(int argc, char ** argv) {
    args::ArgumentParser parser("The client to query SeqOthello from a server. \n");
    args::HelpFlag help(parser, "help", "Display the help menu.", {'h', "help"});
    args::ValueFlag<string> argTranscriptName(parser, "string", "The filename of the transcript fasta file.", {"transcript"});
    args::ValueFlag<string> resultsName(parser, "string", "The filename of the output.", {"output"});
    args::ValueFlag<int>  argServerPort(parser, "int", "The port number of the SeqOthello Server.", {"port"});
    args::ValueFlag<string>  argServerAdd(parser, "string", "Start a SeqOthello Server at address (default: localhost).", {"server"});
    args::Flag   argInteractive(parser, "",  "Start interactive CLI.", {"interactive"});
    args::Flag   argContainmentQuery(parser, "",  "Return the total number of k-mer hits.", {"kmer-hit"});
    args::Flag   argCoverageQuery(parser, "",  "Return detailed k-mer presence/absence information for the transcript, limited to one trascript per query.", {"kmer-hit-map"});

    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (args::Help)
    {
        std::cout << parser;
        return 0;
    }
    catch (args::ParseError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    catch (args::ValidationError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    FILE *fin, *fout;
    string helpstr =  "usage: \t Q ATGCATGC..................... : Show Containment query results for transcript.\n"
                      "     : \t D ATGCATGC..................... : Show Coverage query results for transcript.\n"
                      "     : \t H                               : Show help info\n";
    int x = ((int) (argInteractive)) + ((int) argContainmentQuery) + ((int) argCoverageQuery);
    if (x != 1) {
        std::cerr << " must be one of --interactive, --containment, --coverage"<< std::endl;
        return 1;
    }
    string query_type;
    if (!argInteractive) {
        if (!(argTranscriptName && resultsName && argServerPort)) {
            std::cerr << "must specify args --transcript, --output, --port" << std::endl;
            return 1;
        }
#ifdef __APPLE__
        fin = fopen(args::get(argTranscriptName).c_str(),"rb");
#else
        fin = fopen64(args::get(argTranscriptName).c_str(),"rb");
#endif
        string fnameout = args::get(resultsName);
        fout = fopen(fnameout.c_str(), "w");

        if (fin == NULL) {
            std:: cerr << "Error reading file " << args::get(argTranscriptName) << std::endl;
            return 1;
        }
        if (argContainmentQuery) query_type = TYPE_CONTAINMENT;
        if (argCoverageQuery) query_type = TYPE_COVERAGE;
    }
    else {
        if ((!argServerPort) || argTranscriptName || resultsName) {
            std::cerr << "wrong arg" << std::endl;
            return 1;
        }
        fin = stdin;
        fout = stdout;
        fprintf(fout,"%s", helpstr.c_str());
    }


    char buf[1048576];
    memset(buf,0,sizeof(buf));

    try {
        string servadd = "localhost";
        if (argServerAdd) servadd = args::get(argServerAdd);
        TCPSocket sock(servadd.c_str(), args::get(argServerPort));
        printf("Connecting to %s : %d\n", servadd.c_str(), args::get(argServerPort));
        while ( fgets(buf,sizeof(buf),fin)!= NULL) {
            char * p, *p0;
            p0 = p = &buf[0];
            if (argInteractive) {
                if ((buf[0] != 'Q' && buf[0]!='D') || buf[1]!=' ') {
                    printf("%s\n",helpstr.c_str());
                    continue;
                }
                p0 = p = &buf[2];
                if (buf[0] == 'Q')
                    query_type = TYPE_CONTAINMENT;
                if (buf[0] == 'D')
                    query_type = TYPE_COVERAGE;
            }

            if (*p !='A' && *p!='T' && *p !='G' && *p != 'C') continue;
            while (*p == 'A' || *p == 'T' || *p == 'G' || *p == 'C' || *p == 'N') p++;
            *p = '\0';
            if (strlen(buf)<=3) break;
            int strl;
            sock.sendmsg(query_type);
            sock.sendmsg(p0, strl = strlen(p0));
            printf("Sent a query %s with %d bases.\n", query_type.c_str(), strl );
            string buf;
            int tot = 0 ;
            while (sock.recvmsg(buf)) {
                if (buf.size()==0) break;
                fprintf(fout, "%s\n", buf.c_str());
                tot += buf.size();
                printf("received reponse of %d Bytes.\n", tot);
            }
            if (!argInteractive)
                printf("received reponse of %d Bytes.\n", tot);
        }
        sock.sendmsg("");
    }
    catch(std::runtime_error e) {
        fclose(fin);
        fclose(fout);
        cerr << e.what() << endl;
        exit(1);
    }
    fclose(fin);
    fclose(fout);




}
