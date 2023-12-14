#include "iostream"
#include "string"
#include "fstream"
#include "sstream"
#include "bitset"

int main()
{
    std::ifstream input_file("obj_file.txt");
    std::ofstream output_file("loader.txt");

    if(!input_file.is_open())
    {
        std::cerr << "Could not open the object file\n";
        std::exit(0);
    }
    if(!output_file.is_open())
    {
        std::cerr << "Could not open the output file\n";
        std::exit(0);
    }

    std::string line;
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

                next_target = to_stop;
            }
        }
        else
            continue;
    }
}