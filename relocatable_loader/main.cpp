#include "iostream"
#include "string"
#include "vector"
#include "fstream"
#include "sstream"

int main()
{
    std::ifstream input_file("input.txt");
    std::ofstream output_file("output.txt");


    std::string line;

    std::vector<std::string> modification_lines{};
    std::vector<std::string> text_records{};
    std::string header_record{};

    while(std::getline(input_file, line))
    {
        if(line[0] == 'M')
            modification_lines.push_back(line);
        else if(line[0] == 'T')
            text_records.push_back(line);
        else if(line[0] == 'H')
            header_record = line;
    }

    input_file.close();
    input_file.open("input.txt");

    std::string current_mod_rec{};
    std::string current_mod_address{}, current_mod_size{};

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
                size_t cur_val, add = 0x10;

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

    output_file << header_record << '\n';

    for(size_t i = 0; i < text_records.size(); ++i)
        output_file << text_records[i] << '\n';
}