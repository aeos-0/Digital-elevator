/*
Fix bug reminder block:
    Check over one last time then upload to github and start work on game

*/

//IMPORTANT SELF REMINDER
//This is a reminder to check when I need to use references in for loops (almost always) because I had a bug for days because of it



#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <limits>
#include <chrono>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <queue>
#include <mutex>


std::string logs{};
std::mutex newMutex;

bool isNumber(const std::string words) {
    for (const char letter : words) {
        if (std::isdigit(letter) == 0) {
            return false;
        }
    }
    return true;
}

struct Job {
    int currentFloor{ 1 };
    std::string name;
    int destination{ 1 };
};


struct Vator {
    int pos{ 1 };
    bool isReady{ true };
    int id{};
    //Queue of jobs for each elevator
    std::queue<Job> vatorQueue;

    void goTo(const Job& job) {
        //While current floor != where u have to go, increment by one and move on the loop
        while (this->pos != job.destination) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            if (this->pos > job.destination) {
                this->pos -= 1;
                logs += "Elevator has moved " + job.name + " down a floor\n";
            }
            else {
                this->pos += 1;
                logs += "Elevator has moved " + job.name + " up a floor\n";
            }
        }
        logs += "Floor has been reached and the job is completed\n";
    }

    void addJob(const Job& newJob) {
        //Add job to queue for each elevator
        //Doesn't have to be ordered as I initially thought
        this->vatorQueue.push(newJob);
    }

    void completeJobs() {
        //Mutex probably not needed
        std::lock_guard<std::mutex> lock(newMutex);

        //While the queue is still full do work
        while (!this->vatorQueue.empty()) {
            //Head to the floor of the first job in the queue
            this->goTo(vatorQueue.front());
            //When the job is complete take the first item off the queue and continue
            this->vatorQueue.pop();
        }
        //Since the loop is done the elevator is now ready for more jobs
        this->isReady = true;
    }
};

std::vector<Vator> vatorList;
std::vector<Job> jobList;

//A job has been added to the worker thread
//Big for loop for each job
void passengerSearch() {
        //Go through each job and assign it to an elevator
        for (Job &job : jobList) {
            bool assigned{ false };
            //Check if any elevators are not in use
            for (Vator &vator : vatorList) {
                if (vator.isReady) {
                    //Make sure the program knows the elevator is in use
                    vator.isReady = false;
                    assigned = true;
                    //Job is added to vator queue via function
                    vator.addJob(job);

                    //Lets the elevator complete its jobs on another thread
                    //Problem lies below
                    std::thread doWork(&Vator::completeJobs, &vator);
                    doWork.detach();

                    // Remove the current "job" from "jobList" vector
                    //Code might need review
                    jobList.erase(std::remove_if(jobList.begin(), jobList.end(),
                        [&job](const Job& j) {
                            return &j == &job;
                        }), jobList.end());
                    break;
                }
            }
            
            //If the job was assigned successfully move onto the next
            if (assigned) 
                continue;

            /*If all of the elevators currently have a job see which is the closest to whatever floor
            Grab floor num from the job
            Then check if it can fit the people on it         
            First run algorithm to see what is closest*/

            int reference = job.currentFloor;
            std::vector<int> numbers = {};
            //Of the last queue element of jobs take that jobs destination number in a vector to be calculated
            for (const Vator vator : vatorList) {               
                //Add destination number to vector from last queue item
                numbers.push_back(vator.vatorQueue.back().destination);
            }

            auto closestIt = std::min_element(numbers.begin(), numbers.end(),
                [&](int a, int b) {
                    int diffA = std::abs(a - reference);
            int diffB = std::abs(b - reference);
            return diffA < diffB;
                });

            if (closestIt != numbers.end()) {
                std::cout << "Closest number: " << *closestIt << std::endl;
                //Get elevator with that number (closestIt)
                for (Vator &vator : vatorList) {
                    if (vator.vatorQueue.back().destination == *closestIt) {
                        //Add a job to that elevator
                        vator.addJob(job);

                        // Remove the current "job" from "jobList" vector
                        //Code might need review
                        jobList.erase(std::remove_if(jobList.begin(), jobList.end(),
                            [&job](const Job& j) {
                                return &j == &job;
                            }), jobList.end());
                        break;
                    }
                }
            }
            else {
                std::cout << "Error: Vector is empty.\nEnding program." << std::endl;
                exit(1);
            }
        }
    }


int main() { 
    bool notDone{ true };
    int floor{ 1 }; //The max floors will be 10 so values will be 1-10
    std::string groupName{};
    int destination{};

    std::cout << "<----------Welcome to the elevator program by Aeos!!---------->" << std::endl;
    std::cout << "How many elevators would you like to be generated in this instance?" << std::endl << "If an invalid value is selected 2 will be the default." << std::endl;
    std::string num;
    std::cin >> num;

    //Set up elevator numbers
    if (isNumber(num) && stoi(num) > 1) {
        //If it is a number pick the value the user specified
        std::cout << "Loading " << num << " elevators..." << std::endl;

        for (int i = 0; i < stoi(num); i++) {
            Vator elevator;
            elevator.id = i;
            vatorList.push_back(elevator);
        }

    }
    else {
        //If its not a number pick 2 by default
        std::cout << "Loading 2 elevators..." << std::endl;
        for (int i = 0; i < 2; i++) {
            Vator elevator;
            elevator.id = i;
            vatorList.push_back(elevator);
        }
        num = "2";
    }

    //Setup prompts
    std::cout << "The program is now ready to accept requests!!" << std::endl;
    std::cout << "If any values are invalid they will be handled for you." << std::endl;
    std::cout << "To begin, enter the floor number you wish to start a request on." << std::endl;
    std::cout << "The " << num << " elevators you selected will handle simultaneous requests." << std::endl;

    do {
        std::cout << "<-------------------------------------------------------------------->\n" << std::endl;
        std::cout << "Select your starting floor, it starts at 1 and goes up to 10..." << std::endl;

        std::cin >> floor;
        //If number value invalid just make it 1
        if (floor > 10 || floor < 1)
            floor = 1;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::cout << "What would you like the name of your group to be?" << std::endl;
        std::cin >> groupName;

        std::cout << "What floor number is your destination?" << std::endl;
        std::cin >> destination;
        if (destination == floor) {
            std::cout << "Your destination is the same as you are currently on, so we'll pick for you" << std::endl;
            destination += 1;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Okay " << groupName << ", an elevator will be on its way!" << std::endl;

        //Assign values to job and add to job list
        Job newJob;
        newJob.currentFloor = floor;
        newJob.name = groupName;
        newJob.destination = destination;
        jobList.push_back(newJob);
        
        //Check if they want to quit if not then leave the variable as is
        passengerSearch();

        //Check if the user wants to see the log results
        std::cout << "Do you want to quit the program and see the results now or would you like to add another task?" << std::endl;
        std::cout << "Please type 'y' or 'n' (an invalid value will continue the program)" << std::endl;
        std::string response;
        std::cin >> response;


        //Y or invalid value continues the program while 'n' stops it and shows the results
        if (response == "n" || response == "N") {

            //Check if all elevators are ready so the program can exit
            for (const Vator vator : vatorList) {
                bool allDone = false;
                //If the elevator is not ready then keep checking it until it is
                while (!allDone) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    if (!vator.isReady)
                        continue;
                    else
                        allDone = true;
                }
            }

            std::cout << "The program will now end, here's what happened behind the scenes: " << std::endl;
            std::cout << logs;
            notDone = false;

        }
    } while (notDone);
}