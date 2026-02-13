#include <random>
#include <iostream>
#include <queue>
#include <vector>
#include <cmath>
#include <string>
#include <ctime>
#include <iomanip>

int main(int, char *argv[])
{
    int seed = static_cast<int>(std::time(nullptr));
    std::mt19937 generator(seed);

    // the arguments from command line are stored in arg every argument is separated by space
    // arg[0] is the name of the program
    // for every i in Pi, Qi, MUi the argument is stored in order and not as an array
    // therefore we need to build the vectors from the arguments by a loop
    // we access the queues by queues[i] where i is the index of the queue

    // after building the vectors we can proceed with the simulation
    // we need to create events based on the parameter T and lambda by randomin a poisson distribution of parameter lambda over the time T
    // we need to create M queues
    // we need to create a function to process the events
    // for each event we need to callculate the choice of the queue based on the probability of Pi
    // we need to callculate the service time based on the parameter MUi
    // we need to stop exepting request after the time T and to process the ones that are already in the queue
    // we need to keep track of the total number of requests served and denied
    // the output is a line of string with numbers separated by space of:
    // number of requests served, number of requests denied, time of the last request fully proccesed, average waiting time, average service time.

    int completed_requests = 0;
    int denied_requests = 0;
    double last_completion_time = 0.0;
    double total_waiting_time = 0.0;
    double total_service_time = 0.0;

    int i = 1;
    int T = std::stoi(argv[i++]);
    int M = std::stoi(argv[i++]);
    if (M <= 0)
    {
        std::cerr << "Error: servers size must be grater then zero." << std::endl;
        return 1;
    }
    std::vector<double> Pi;
    for (int argg = 0; argg < M; argg++)
    {
        if (std::stoi(argv[i]) < 0)
        {
            std::cerr << "Error: Queue_probabilitys must be non-negative." << std::endl;
            return 1;
        }
        Pi.push_back(std::stod(argv[i++]));
    }

    double lambda = std::stod(argv[i++]);
    std::vector<int> Qi;
    for (int argg = 0; argg < M; argg++)
    {
        if (std::stoi(argv[i]) < 0)
        {
            std::cerr << "Error: Queue size must be non-negative." << std::endl;
            return 1;
        }
        Qi.push_back(std::stoi(argv[i++]) + 1);
    }
    std::vector<double> MUi;
    for (int argg = 0; argg < M; argg++)
    {
        if (std::stoi(argv[i]) < 0)
        {
            std::cerr << "Error: Service_probabilitis must be non-negative." << std::endl;
            return 1;
        }
        MUi.push_back(std::stod(argv[i++]));
    }

    double sum_Pi = 0.0;
    for (double p : Pi)
    {
        sum_Pi += p;
    }
    if (std::abs(sum_Pi - 1.0) > 1e-6) // allow for floating point precision issues rounding to 1
    {
        std::cerr << "Error: Sum of probabilities must be greater than one." << std::endl;
        return 1;
    }

    std::vector<std::queue<double>> queues; // vector of M queues
    for (int i = 0; i < M; i++)
    {
        queues.push_back(std::queue<double>()); // create empty queue
    }

    double current_time = 0.0;
    std::discrete_distribution<int> dist(Pi.begin(), Pi.end()); // to choose the server based on Pi
    std::vector<double> last_departure_time(M, 0.0);
    while (current_time <= T)
    {
        // Generate next arrival time using exponential distribution
        std::exponential_distribution<double> arrival_dist(1 / lambda);
        double inter_arrival_time = arrival_dist(generator);
        current_time += inter_arrival_time;

        if (current_time > T)
        {
            break;
        }
        int server_index = dist(generator); // return the index of the server chosen based on Pi
        double service_rate = MUi[server_index];
        std::exponential_distribution<double> service_dist(1 / service_rate); // to generate service time
        double service_time = service_dist(generator);                        // generate service time
        double arrival_time = current_time;
        double service_start_time = std::max(arrival_time, last_departure_time[server_index]);
        double departure_time = service_time + service_start_time;
        double waiting_time = departure_time - arrival_time - service_time;

        if (queues[server_index].size() < static_cast<size_t>(Qi[server_index]))
        {
            // enqueue the arrival time
            queues[server_index].push(current_time);
            last_departure_time[server_index] = departure_time;
            total_waiting_time += waiting_time;
            total_service_time += service_time;
            completed_requests++;
        }
        else
        {
            // request denied
            denied_requests++;
        }

        if (departure_time > last_completion_time)
        {
            last_completion_time = departure_time;
        }
    }

    // Iterate over each server
    for (int server_index = 0; server_index < M; ++server_index)
    {
        // Process all jobs currently in this server's queue
        while (!queues[server_index].empty())
        {

            // Pop the next job
            double arrival_time = queues[server_index].front();
            queues[server_index].pop();

            // Compute service start time
            double service_start_time = std::max(arrival_time, last_departure_time[server_index]);

            // Generate service time
            std::exponential_distribution<double> service_dist(1.0 / MUi[server_index]);
            double service_time = service_dist(generator);

            // Compute departure time
            double departure_time = service_start_time + service_time;

            // Update per-server last departure time
            last_departure_time[server_index] = departure_time;

            // Update global statistics
            double waiting_time = service_start_time - arrival_time;
            total_waiting_time += waiting_time;
            total_service_time += service_time;
            completed_requests++;

            // Track last completion across all servers
            if (departure_time > last_completion_time)
            {
                last_completion_time = departure_time;
            }
        }
    }

    std::vector<std::string> output;
    output.push_back(std::to_string(completed_requests));
    output.push_back(std::to_string(denied_requests));
    output.push_back(std::to_string(static_cast<int>(std::ceil(last_completion_time))));
    double average_waiting_time;
    if(completed_requests > 0) {
        average_waiting_time= (total_waiting_time / completed_requests);
    } else {
        average_waiting_time = 0.0;
    }
    double average_service_time;
    if(completed_requests > 0){
        average_service_time=(total_service_time / completed_requests);
    }
    else {
        average_service_time = 0.0;
    }
    std::cout << std::fixed << std::setprecision(4);
    output.push_back(std::to_string(average_waiting_time));
    output.push_back(std::to_string(average_service_time));

    for (size_t idx = 0; idx < output.size(); idx++)
    {
        std::cout << output[idx];
        if (idx != output.size() - 1)
        {    
            std::cout << " ";
        }
    }

    return 0;
}