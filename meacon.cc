#include <iostream>
#include <vector>
#include <deque>
#include <cmath>
#include <string>
#include <fstream>

int main(int argc, char* argv[])
{
  std::string in_name(argv[1]);
  std::string out_name = "meacon_" + in_name;

  int start_time = 180; // Seconds
  int sample_rate = 2600000; // 2.6Mhz

  double power = 0;
  double max_power = 1.1;

  double delay = 0;
  int max_delay = 2600000;
  std::deque<int16_t> delayed_samples;

  // Open the simulated Binary file
  std::ifstream input(in_name, std::ios::binary);

  // Open the output Binary file
  std::ofstream output(out_name, std::ios::binary);

  // Read the binary samples until done
  unsigned long long int i = 0;
  unsigned long long int j = 0;
  unsigned long long int k = 0;

  int16_t sample = 0x00;

  bool alerted_power_srt = false;
  bool alerted_power_max = false;
  bool alerted_delay_srt = false;
  bool alerted_delay_max = false;

  while (input.good())
  {
    input.read(reinterpret_cast<char*>(&sample), sizeof sample);

    i++;

    int16_t out_sample = sample;

    if (delay >= 1) // don't waste ops
      delayed_samples.push_back(sample);

    // We know we don't need to track samples older than the current delay
    while (delayed_samples.size() > delay)
      delayed_samples.pop_front();

    // Spoofing should take effect
    if (i > (start_time * sample_rate * 2))
    {
      j++;

      // Increase power steadily
      if (power < max_power)
      {
        if (!alerted_power_srt)
        {
          std::cout << "Beginning power ramp up at sample " << i / 2
            << std::endl;

          alerted_power_srt = true;
        }
        power = std::log(j / 100000.0 + 1) / 10.0;

        if (power > max_power)
        {
          power = max_power;

          if (!alerted_power_max)
          {
            std::cout << "Reached max power at sample " << i / 2 << std::endl;

            alerted_power_max = true;
          }
        }
      }

      // Once power is maxed out, increase delay steadily
      if (power == max_power)
      {
        k++;

        if (delay < max_delay)
        {
          if (!alerted_delay_srt)
          {
            std::cout << "Beginning delay ramp up at sample " << i / 2
              << std::endl;

            alerted_delay_srt = true;
          }

          // Log delay - why??
          // delay = std::log(k / 100000.0 + 1) / 10.0;
          delay += 1 / 300.0;

          if (delay > max_delay)
          {
            delay = max_delay;

            if (!alerted_delay_max)
            {
              std::cout << "Reached max delay at sample " << i / 2 << std::endl;

              alerted_delay_max = true;
            }
          }

        }
      }

      if (!delayed_samples.empty())
        out_sample += delayed_samples.front() * power;
      else
        out_sample *= (1 + power);
    }

    // Write to file
    output.write(reinterpret_cast<char*>(&out_sample), sizeof out_sample);
  }

  std::cout << "Finished processing " << i / 2 << " samples" << std::endl;
  std::cout << "Maximum delay reached: " << delay << " samples" << std::endl;

  if (output.is_open())
    output.close();
  else
    std::cerr << "Some serious error, outfile not opened" << std::endl;

  if (input.is_open())
    input.close();
  else
    std::cerr << "Some serious error, infile not opened" << std::endl;

}
