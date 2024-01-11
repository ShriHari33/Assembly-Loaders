#include "iostream"
#include "string"
#include "vector"
#include "map"
#include "algorithm"
#include "fstream"
#include "sstream"
#include "cstring"
#include "iomanip"

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

    size_t getSymbolAddress(std::string symbol_name)
    {
        for(auto e : external_sym_table)
        {
            auto it = std::find_if(e.reference_list.begin(), e.reference_list.end(), [symbol_name](const std::pair<std::string, size_t>& p) { return p.first == symbol_name; });
            if(it != e.reference_list.end())
                return it->second;
        }
        return -1;

    }
};


int main()
{
    std::ifstream input_file("input.txt");
    if(!input_file.is_open())
    {
        std::cerr << "Error opening input file" << std::endl;
        return 1;
    }

    size_t CSADDRESS = 0;
    size_t CSLENGTH = 0;
    size_t PROGADDR = 0;

    std::cout << "What did the OS give you? ";
    std::cin >> std::hex >> PROGADDR;
    CSADDRESS = PROGADDR;
    std::cout << "\n\n";

    std::string line;
    symbolTable external_sym_table;
    std::vector<std::string> mod_records{};
    //std::vector<std::vector<std::string>> text_records;
    size_t count = 0;

    std::ofstream output_file2("output2.txt");
    if(!output_file2.is_open())
    {
        std::cerr << "Error opening output2 file" << std::endl;
        return 1;
    }
    output_file2 << std::string(23, '\n');

    std::string end_record_to_be_appended{};
    std::string header_record_to_be_appended{};

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
                                                << std::hex << std::showbase << external_sym_table.getCSAddress(csect_name) << std::endl;

                no_print = true;
            }
            else
            {
                external_sym_table.add_CS(csect_name, CSADDRESS, CSLENGTH);

                if(count == 0)
                    header_record_to_be_appended = line;
            }
        }
        else if(line.empty())
            continue;

        while( std::getline(input_file, line) )
        {
            // need these below conditions to handle "blank" lines in each control section if any. We need to
            // essentially break if we found the END record, but we also need to continue if we found a blank line
            // because doing line[0] == 'E' will be a SEGFAULT if the line is empty
            if(line.empty())
                continue;
            else if(line[0] == 'E')
            {
                // we only append the first end record to the output file. All others are bogus
                if(count == 0)
                    end_record_to_be_appended = line;

                count++;
                break;
            }

            // main logic
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
            else if(line[0] == 'T')
            {
                // doesn't matter which CSECT text records we are modifying, we will have to modify the start address of it
                std::string need_for_cur_line_to_modify{line};
                std::string address(line, 2, 6);

                // get the current address
                size_t cur_text_rec_address = std::stoi(address, nullptr, 16);

                // modify the address by adding the address of the current CSECT
                cur_text_rec_address += CSADDRESS;

                // ensuring that it has exactly 6 characters
                std::ostringstream oss;
                oss << std::setw(6) << std::setfill('0') << std::hex << cur_text_rec_address;

                need_for_cur_line_to_modify.replace(2, 6, oss.str());

                output_file2 << need_for_cur_line_to_modify << '\n';
            }
            else if(line[0] == 'M')
            {
                // push the modification record by updating its address in the whole scheme of the program
                std::string temp = line;
                std::string mod_record_addr(line, 2, 6);
                size_t mod_record_address = std::stoi(mod_record_addr, nullptr, 16);
                mod_record_address += CSADDRESS;

                std::ostringstream oss;
                oss << std::setw(6) << std::setfill('0') << std::hex << mod_record_address;

                temp.replace(2, 6, oss.str());
                mod_records.push_back(temp);
            }
        }
        CSADDRESS += CSLENGTH;
    }

    //external_sym_table.print();

    // now we need to modify the end record we stored for the first CSECT. I do it in a different scope because I don't want
    //    // to use the same variable names (oss is reserved for temporary oss objects)
    {
        std::ostringstream oss;
        oss << std::setw(6) << std::setfill('0') << std::hex << PROGADDR;
        end_record_to_be_appended.replace(2, 6, oss.str());
    }
    output_file2 << end_record_to_be_appended << '\n';


    // now we need to modify the header record of the first CSECT. I do it in a different scope because I don't want
    // to use the same variable names (oss is reserved for temporary oss objects)
    {
        std::ostringstream oss;
        oss << std::setw(6) << std::setfill('0') << std::hex << (CSADDRESS - PROGADDR);
        header_record_to_be_appended.replace(16, 6, oss.str());

        oss.clear();
        oss.str("");

        oss << std::setw(6) << std::setfill('0') << std::hex << PROGADDR;
        header_record_to_be_appended.replace(9, 6, oss.str());
    }
    output_file2.seekp(0, std::ios::beg);
    output_file2 << header_record_to_be_appended << '\n';

    input_file.close();
    output_file2.close();


    std::fstream final_file("output2.txt", std::ios::in | std::ios::out);
    std::streampos prev_tracker = final_file.tellg();
    std::streampos cur_tracker = prev_tracker;

    std::map<size_t, std::streampos> text_record_map;
    while(std::getline(final_file, line))
    {
        cur_tracker = final_file.tellg();
        if(line[0] == 'T')
        {
            std::string address(line, 2, 6);
            size_t cur_text_rec_address = std::stoi(address, nullptr, 16);
            text_record_map[cur_text_rec_address] = prev_tracker;
        }
        prev_tracker = cur_tracker;
    }

    for(auto& e : text_record_map)
    {
        std::cout << std::hex << e.first << ' ' << std::dec << e.second << std::endl;
    }

    // ok cool, it works. Now we need to do pass 2

    final_file.clear();
    final_file.seekg(0, std::ios::beg);

    size_t EXECADDR = 0;
    CSADDRESS = PROGADDR;
    bool first_header = true;

    for(size_t i = 0; i < mod_records.size(); ++i)
    {
        // we now need to check where the modification record's address demands to be changed in the file
        std::string mod_record_addr(mod_records[i], 2, 6);
        size_t mod_record_address = std::stoi(mod_record_addr, nullptr, 16);
        size_t mod_rec_size = std::stoi(std::string(mod_records[i], 9, 2), nullptr, 16);
        std::string symbol = std::string(mod_records[i].begin() + 13, mod_records[i].end());
        bool add = (mod_records[i][12] == '+');

        size_t sym_address = external_sym_table.getSymbolAddress(symbol);
        if(sym_address == -1)
        {
            std::cerr << "Symbol " << symbol << " not found in the external symbol table" << std::endl;
            no_print = true;
            break;
        }

        // we need to check where this address now falls in the map
        auto it = text_record_map.upper_bound(mod_record_address);
        std::map<size_t, std::streampos>::iterator prev_it;

        if(it == text_record_map.end())
            prev_it = --text_record_map.end();
        else
            prev_it = std::prev(it);

        std::streampos position = prev_it->second;
        position += 12; // we go to the main part, skipping all the text record meta-data
        size_t diff_bytes = mod_record_address - prev_it->first;

        final_file.seekg(position, std::ios::beg);

        // now we need to move to that position of the address (written in text), and modify the record properly
        while(diff_bytes != 0)
        {
            final_file.seekg(2, std::ios::cur);

            if(final_file.peek() == '_')
                final_file.seekg(1, std::ios::cur);

            diff_bytes--;
        }

        // now we are at the correct position, we need to modify the record with the proper size of half-bytes
        // stored above in the variable mod_rec_size

        std::streampos need_later = final_file.tellg();
        std::string temp;
        std::getline(final_file, temp, '_');
        size_t cur_addr = 0;
        //std::cout << "llflwf" + temp << std::endl;

        std::istringstream iss(temp);
        iss >> std::hex >> cur_addr;

        add ? (cur_addr += sym_address) : (cur_addr -= sym_address);

        std::ostringstream oss;
        oss << std::setw(temp.size()) << std::setfill('0') << std::hex << cur_addr;

        final_file.seekg(need_later, std::ios::beg);
        final_file << oss.str();
    }

    final_file.close();
    return 0;
}