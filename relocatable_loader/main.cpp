#include "iostream"
#include "string"
#include "vector"
#include "fstream"
#include "sstream"
#include "bitset"

int main()
{
    std::ifstream input_file("input.txt");
    std::ofstream output_file("output.txt");


    std::string line;

    std::vector<std::string> modification_lines{};
    std::vector<std::string> text_records{};
    std::string header_record{};
    std::string end_record{};

    while(std::getline(input_file, line))
    {
        if(line[0] == 'M')
            modification_lines.push_back(line);
        else if(line[0] == 'T')
            text_records.push_back(line);
        else if(line[0] == 'H')
            header_record = line;
        else
            end_record = line;
    }

    input_file.close();
    input_file.open("input.txt");

    std::string current_mod_rec{};
    std::string current_mod_address{}, current_mod_size{};
    size_t add{};

    std::cout << "Enter the start address to be used: ";
    std::cin >> std::hex >> add;

    for(size_t i = 0; i < modification_lines.size(); ++i)
    {
        current_mod_rec = modification_lines[i];

        // trim the current_mod_rec to just allow for the address and the size
        current_mod_address = current_mod_rec.substr(2, 6);
        //std::cout << "cur mod address: " << current_mod_address << '\n';

        current_mod_size = current_mod_rec.substr(9, 2);
        //std::cout << "cur mod size: " << current_mod_size << '\n';

        size_t mod_add = std::strtol(current_mod_address.c_str(), NULL, 16);
        //std::cout << mod_add << '\n';

        size_t mod_size = std::strtol(current_mod_size.c_str(), NULL, 16);
        //std::cout << mod_size << '\n';

        size_t text_rec_index = 0;
        while(text_rec_index != text_records.size())
        {
            line = text_records[text_rec_index];
            size_t cur_add = std::strtol(line.substr(2, 6).c_str(), NULL, 16);
            //std::cout << "cur text address start add: " << cur_add << '\n';

            size_t fit = std::strtol(line.substr(9, 2).c_str(), NULL, 16);
            //std::cout << "cur text address len: " << fit << '\n';

            size_t index = 11;

            // check if our mod_address falls in the current range of the text record
            if( mod_add <= (cur_add + fit) )
            {
                // std::cout << "ami?\n";
                size_t iter_bytes_count = 0;
                size_t iter = 0;
                /*
                 * relative checking.
                 * this is done to see "how" many bytes we need to move forward
                */
                while(iter_bytes_count != (mod_add - cur_add))
                {
                    iter += 2;
                    // std::cout << "at: " << line[index + iter] << '\n';

                    // if the front guy is an "_", we just skip it because we don't want to mess up the count
                    if(line[index + iter + 1] == '_')
                    {
                        //std::cout << "at: " << line[index + iter] << '\n';
                        iter++;
                    }

                    iter_bytes_count++;
                }

                //std::cout << "Iter: " << iter << '\n';

                size_t next_underscore = line.find('_', index + iter);
                //std::cout << "next under: " << next_underscore << '\n';
                std::string substring{};

                if(next_underscore != std::string::npos)
                    substring = line.substr(index + iter + 1, next_underscore - (index + iter + 1));
                else
                    substring = line.substr(index + iter + 1, (line.length()  - (index + iter + 1))), std::cout << "s: " << substring << '\n';

                //std::cout << "substr: " << substring << '\n';
                std::string needed_substring(substring, substring.length() - mod_size, mod_size);
                // std::cout << "n: " << needed_substring << '\n';
                std::istringstream iss(needed_substring);
                size_t cur_val = 0;

                //std::cout << "ADD: " << add << '\n';
                //std::cout << "did i get through?\n";
                iss >> std::hex >> cur_val;
                size_t updated_val = cur_val + add;
                size_t start = (index + iter + 2) - (needed_substring.length() - mod_size);

                // std::cout << "st: " << start << '\t' << "it: " << iter << '\n';
                //std::cout << "did i get through?\n";
                std::ostringstream oss;
                oss << std::hex << updated_val;

                std::string temp{oss.str()};

                while(temp.length() != needed_substring.length())
                    temp = '0' + temp;

                int ii = 0;
                for(size_t k = start; k < start + mod_size; ++k)
                    line[k] = temp[ii++];

                text_records[text_rec_index] = line;

                //std::cout << "inserted at: " << text_rec_index << "THIS " << line << '\n';
                break;
            }
            text_rec_index++;
        }
    }

    // need to modify the Header, Text and End records address part too, as the addition of the new OS given address should happen

    // for header record
    {
        std::istringstream iss(header_record.substr(9, 6));
        size_t cur_header_add_begins_from{}, should_begin_from{};
        iss >> std::hex >> cur_header_add_begins_from;

        should_begin_from = cur_header_add_begins_from + add;

        std::ostringstream oss;
        oss << std::hex << should_begin_from;

        std::string should_begin_from_str{oss.str()};
        while(should_begin_from_str.length() != 6)
            should_begin_from_str = '0' + should_begin_from_str;

        header_record.replace(9, 6, should_begin_from_str);
    }

    // for text records
    for(size_t i = 0; i < text_records.size(); ++i)
    {
        std::istringstream iss(text_records[i].substr(2, 6));
        size_t cur_text_add_begins_from{}, should_begin_from{};
        iss >> std::hex >> cur_text_add_begins_from;

        should_begin_from = cur_text_add_begins_from + add;

        std::ostringstream oss;
        oss << std::hex << should_begin_from;

        std::string should_begin_from_str{oss.str()};
        while(should_begin_from_str.length() != 6)
            should_begin_from_str = '0' + should_begin_from_str;

        text_records[i].replace(2, 6, should_begin_from_str);
    }

    // for end record

    {
        std::istringstream iss(end_record.substr(2, 6));
        size_t cur_end_add_begins_from{}, should_begin_from{};
        iss >> std::hex >> cur_end_add_begins_from;

        should_begin_from = cur_end_add_begins_from + add;

        std::ostringstream oss;
        oss << std::hex << should_begin_from;

        std::string should_begin_from_str{oss.str()};
        while(should_begin_from_str.length() != 6)
            should_begin_from_str = '0' + should_begin_from_str;

        end_record.replace(2, 6, should_begin_from_str);
    }

    // now finally output all of them to the output file
    output_file << header_record << '\n';

    for(size_t i = 0; i < text_records.size(); ++i)
        output_file << text_records[i] << '\n';

    output_file << end_record << '\n';

    // close the currently opened file, and open the output.txt file so that we can use that as an input
    // to generate the byte by byte code

    input_file.close();

    // similarly, close the output file, as we need to open that as an input now
    output_file.close();

    input_file.open("output.txt");
    output_file.open("final.txt");

    if(!input_file.is_open() || !output_file.is_open())
    {
        std::cerr << "gg go next\n";
        exit(1);
    }

    std::string name_of_prog, start_addr, length_of_prog;
    std::string start_addr_for_text_record;

    while(std::getline(input_file, line))
    {
        if(line[0] == 'H')
        {
            name_of_prog = line.substr(2, 6);
            start_addr = line.substr(9, 6);
            length_of_prog = line.substr(16, 6);
        }
        else if(line[0] == 'T')
        {
            start_addr_for_text_record = line.substr(2, 6);
            bool flag_exit_text_rec = false;
            size_t next_target = 11;
            size_t next_addr = 0;

            while(!flag_exit_text_rec)
            {
//                std::istringstream addr_resolution(start_addr_for_text_record);
//                addr_resolution >> std::hex >> next_addr;

                next_target = line.find('_', next_target);

                if(next_target == std::string::npos)
                {
                    flag_exit_text_rec = true;
                    continue;
                }

                int it = next_target + 1;
                size_t to_stop = line.find('_', next_target + 1);

                if(to_stop == std::string::npos)
                    flag_exit_text_rec = true, to_stop = line.length(); //

                /*
                 * may happen that it might skip the to_stop, so might have to use it < to_stop
                 */
                while(it != to_stop)
                {
                    while(start_addr_for_text_record.length() < 6)
                        start_addr_for_text_record = '0' + start_addr_for_text_record;

                    std::string to_put_hex = std::string(line, it, 2);
                    std::string to_put_bin;
                    // std::cout << to_put_hex[0] << '\t';main
                    for(int i = 0; i < 2; ++i)
                    {
                        int get_hex_bit;
                        std::string temp;
                        temp += to_put_hex[i];
                        std::istringstream temp_iss(temp);

                        //std::cout << "PASSING " << std::to_string(to_put_hex[i]) << '\n';
                        temp_iss >> std::hex >> get_hex_bit;

                        to_put_bin += std::bitset<4>(get_hex_bit).to_string();
                    }

                    output_file << start_addr_for_text_record << '\t' << to_put_hex << '\t' << to_put_bin << '\n';

                    // will only output the bytes in hex, not in binary
                    //output_file << start_addr_for_text_record << '\t' << to_put_hex << '\n';
                    std::istringstream addr_resolution(start_addr_for_text_record);
                    addr_resolution >> std::hex >> next_addr;
                    next_addr += 1;

                    std::ostringstream oss;
                    oss << std::hex << next_addr;

                    start_addr_for_text_record = oss.str();
                    oss.clear();
                    oss.str("");
                    it += 2;
                }
                output_file << '\n';

                next_target = to_stop;
            }
            output_file << "\n\n";
        }
        else
            continue;
    }
}