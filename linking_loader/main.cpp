#include "iostream"
#include "string"
#include "vector"
#include "map"
#include "algorithm"
#include "fstream"
#include "sstream"
#include "cstring"

class entry
{
public:
    std::string csect_name;
    size_t csect_address;
    std::vector<std::pair<std::string, size_t>> reference_list;
    size_t csect_length;
};

bool no_print = false;
class symbolTable
{
private:
    std::vector<entry> external_sym_table;

public:
    symbolTable()
    {
        external_sym_table.clear();
    }

    bool search_CS(std::string csect_name)
    {
        auto it = std::find_if(external_sym_table.begin(),
                               external_sym_table.end(),
                               [csect_name](const entry& e) { return e.csect_name == csect_name; });

        if(it == external_sym_table.end())
            return false;

        return true;

    }

    bool search_entry(std::string def_rec)
    {
        /*
         * need to find if the def_rec is already present in the external symbol table. the def_rec is the reference_list
         * of the entry class. if it is, then we return false because duplicate
        */

        for(auto e : external_sym_table)
        {
            auto it = std::find_if(e.reference_list.begin(), e.reference_list.end(), [def_rec](const std::pair<std::string, size_t>& p) { return p.first == def_rec; });
            if(it != e.reference_list.end())
                return true;
        }

        return false;
    }

    void add_CS(std::string csect_name, size_t csect_address, size_t csect_length)
    {
        auto it = std::find_if(external_sym_table.begin(),
                                          external_sym_table.end(),
                                         [csect_name](const entry& e) { return e.csect_name == csect_name; });

        external_sym_table.push_back({csect_name, csect_address, {}, csect_length});
    }

    void add_entry(std::string csect_name, std::string symbol_name, size_t symbol_address)
    {
        auto it = std::find_if(external_sym_table.begin(),
                               external_sym_table.end(),
                               [csect_name](const entry& e) { return e.csect_name == csect_name; });

        it->reference_list.push_back({symbol_name, symbol_address});
    }

    void print()
    {
        write("listing.txt");
        if(no_print)
        {
            std::cout << "\n\nWill not print the symbol table because of errors\nBut I will write it to a file\n\n";
            return;
        }

        // std::cout << "\n\nEXTERNAL SYMBOL TABLE\n\n";
        std::cout << "\n\nCSECT\tSYMBOL\tADDRESS\tLENGTH\n\n";
        for (auto e : external_sym_table)
        {
            std::cout << e.csect_name << "\t\t" << std::hex << ' ' << e.csect_address << '\t'
                                                << std::hex << "  " << e.csect_length << std::endl;

            for (auto r : e.reference_list)
                std::cout << '\t' << r.first << '\t' << std::hex << ' ' << r.second << std::endl;

            std::cout << "-------------------------------" << std::endl << std::endl;
        }
    }

    void write(const char* filename)
    {
        std::ofstream output_file(filename);

        if(!output_file.is_open())
        {
            std::cerr << "Error opening listing file" << std::endl;
            return;
        }

        output_file << "\n\nCSECT\tSYMBOL\tADDRESS\tLENGTH\n\n";
        for (auto e : external_sym_table)
        {
            output_file << e.csect_name << "\t\t" << std::hex << ' ' << e.csect_address << '\t'
                                                << std::hex << "  " << e.csect_length << std::endl;

            for (auto r : e.reference_list)
                output_file << '\t' << r.first << '\t' << std::hex << ' ' << r.second << std::endl;

            output_file << "-------------------------------" << std::endl << std::endl;
        }
    }

    size_t getCSAddress(std::string csect_name)
    {
        auto it = std::find_if(external_sym_table.begin(),
                               external_sym_table.end(),
                               [csect_name](const entry& e) { return e.csect_name == csect_name; });

        if(it == external_sym_table.end())
            return -1;

        return it->csect_address;
    }
};


int main()
{
    std::ifstream input_file("input.txt");
    std::ofstream output_file("output.txt");

    if(!input_file.is_open())
    {
        std::cerr << "Error opening input file" << std::endl;
        return 1;
    }
    if(!output_file.is_open())
    {
        std::cerr << "Error opening output file" << std::endl;
        return 1;
    }

    size_t CSADDRESS = 0;
    size_t CSLENGTH = 0;

    std::cout << "What did the OS give you? ";
    std::cin >> std::hex >> CSADDRESS;
    std::cout << "\n\n";

    std::string line;
    symbolTable external_sym_table;

    while(std::getline(input_file, line))
    {
        std::string csect_name{};

        if(line[0] == 'H')
        {
            // we entered a new CSECT, so we fetch the length of the CSECT
            CSLENGTH = std::stoi(std::string(line, 16, 6), nullptr, 16);

            csect_name = std::string(line, 2, 6);
            bool present_CS = external_sym_table.search_CS(csect_name);

            if(present_CS)
            {
                std::cerr << "Duplicate CSECT " << csect_name << " found. Previously declared at address "
                                                << std::hex << external_sym_table.getCSAddress(csect_name) << std::endl;

                no_print = true;
            }
            else
                external_sym_table.add_CS(csect_name, CSADDRESS, CSLENGTH);
        }

        while(std::getline(input_file, line) && line[0] != 'E')
        {
            if(line[0] == 'D')
            {
                std::string def_record_line(line.begin() + 2, line.end());
                std::istringstream iss(def_record_line);

                std::string cur_def_rec{};
                size_t cur_def_val = 0;

                while(std::getline(iss, cur_def_rec, '_'))
                {
                    bool present_symbol = external_sym_table.search_entry(cur_def_rec);

                    if(present_symbol)
                    {
                        std::cerr << "Duplicate symbol " <<  cur_def_rec << " found in CSECT " << csect_name << std::endl;
                        std::getline(iss, cur_def_rec, '_');
                        no_print = true;
                    }
                    else
                    {
                        // add it to the external symbol table at the current CSECT
                        std::string cur_def_val_str{};
                        std::getline(iss, cur_def_val_str, '_');
                        cur_def_val = std::stoi(cur_def_val_str, nullptr, 16);
                        external_sym_table.add_entry(csect_name, cur_def_rec, cur_def_val + CSADDRESS);
                    }
                }
            }
        }

        CSADDRESS += CSLENGTH;
    }

    external_sym_table.print();
}