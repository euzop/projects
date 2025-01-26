#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <limits>
#include <unordered_map>

using namespace std;

// Constants
const int greenTime = 10;
const int yellowTime = 3;
const int redWaitTimes[4] = {0, 13, 26, 39};
const int numberOfRoads = 4;

// Shared state
mutex mtx;
int vehicleCounts[4] = {0, 0, 0, 0}; // Vehicle counts for each quadrant
bool programRunning = true;          // Flag to control program termination
unordered_map<int, thread::id> quadrantThreadIDs; // Map to store thread IDs for each quadrant

void clearInputStream() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int getValidatedInput(const string &prompt, int minValue, int maxValue) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (cin.fail() || value < minValue || value > maxValue) {
            cout << "Invalid input. Please enter a number between " << minValue << " and " << maxValue << ".\n";
            clearInputStream();
        } else {
            return value;
        }
    }
}

void displayTrafficLight(int currentGreenRoad, bool isYellow) {
    lock_guard<mutex> lock(mtx);
    system("CLS");

    const string green = "G";
    const string yellow = "Y";
    const string red = "R";

    vector<string> quadrants(4, red);
    vector<string> arrows(4, " ");

    if (isYellow) {
        quadrants[currentGreenRoad] = yellow;
    } else {
        quadrants[currentGreenRoad] = green;
    }

    if (currentGreenRoad == 0) {
        arrows[0] = ">";
        arrows[1] = "v";
        arrows[3] = "^";
    } else if (currentGreenRoad == 1) {
        arrows[0] = ">";
        arrows[1] = "v";
        arrows[2] = "<";
    } else if (currentGreenRoad == 2) {
        arrows[1] = "v";
        arrows[2] = "<";
        arrows[3] = "^";
    } else if (currentGreenRoad == 3) {
        arrows[0] = ">";
        arrows[2] = "<";
        arrows[3] = "^";
    }

    cout << "                             |         |         |" << endl;
    cout << "                             |    |    |    |    |" << endl;
    cout << "                             |         |         |" << endl;
    cout << "                             |    |    |    |    |" << endl;
    cout << "                             |         |         |" << endl;
    cout << "                             |    |    4" << quadrants[3] << "   |    |" << endl;
    cout << "------------------------------              " << arrows[3] << "    ------------------------------" << endl;
    cout << "                                            " << arrows[3] << "                                  " << endl;
    cout << "--  --  --  --  --  --  --  --" << arrows[2] << arrows[2] << arrows[2] << "           " << arrows[3] << "    --  --  --  --  --  --  --  --" << endl;
    cout << "                                                                               " << endl;
    cout << "----------------------------3" << quadrants[2] << "                   1" << quadrants[0] << "----------------------------" << endl;
    cout << "                                                                               " << endl;
    cout << "--  --  --  --  --  --  --  --    " << arrows[1] << "           " << arrows[0] << arrows[0] << arrows[0] << "--  --  --  --  --  --  --  --" << endl;
    cout << "                                  " << arrows[1] << "                                                      " << endl;
    cout << "------------------------------    " << arrows[1] << "    2" << quadrants[1] << "        ------------------------------" << endl;
    cout << "                             |    |    |    |    |" << endl;
    cout << "                             |         |         |" << endl;
    cout << "                             |    |    |    |    |" << endl;
    cout << "                             |         |         |" << endl;
    cout << "                             |    |    |    |    |" << endl;
    cout << "                             |         |         |" << endl;

    if (isYellow) {
        cout << "Current Yellow Light: Quadrant " << currentGreenRoad + 1 << "\n\n";
    } else {
        cout << "Current Green Light: Quadrant " << currentGreenRoad + 1 << "\n\n";
    }

    for (int road = 0; road < numberOfRoads; ++road) {
        if (road == currentGreenRoad) {
            if (isYellow) {
                cout << "Quadrant " << road + 1 << ": [YELLOW: " << yellowTime << " Seconds Remaining] \n  - (Vehicles on Queue: " << vehicleCounts[road] << ")\n  - (Thread ID: " << quadrantThreadIDs[road] << ")\n\n";
            } else {
                cout << "Quadrant " << road + 1 << ": [GREEN: " << greenTime << " Seconds Remaining] \n  - (Vehicles on Go: " << vehicleCounts[road] << ")\n  - (Thread ID: " << quadrantThreadIDs[road] << ")\n\n";
            }
        } else {
            int remainingTime = redWaitTimes[(road - currentGreenRoad + numberOfRoads) % numberOfRoads];
            if (isYellow) {
                remainingTime = max(0, remainingTime - greenTime);
            }
            cout << "Quadrant " << road + 1 << ": RED: " << remainingTime << " Seconds Remaining \n  - (Vehicles on Queue: " << vehicleCounts[road] << ") \n  - (Thread ID: " << quadrantThreadIDs[road] << ")\n\n";
        }
    }
    cout << "\n" << endl;
}

void vehicleUpdater(int &currentGreenRoad, int quadrantID) {
    quadrantThreadIDs[quadrantID] = this_thread::get_id();
    int redCounter = 0; // Counter for adding vehicles to red quadrants
    int greenCounter = 0; // Counter for adding vehicles to the green quadrant

    while (programRunning) {
        this_thread::sleep_for(chrono::seconds(1));

        lock_guard<mutex> lock(mtx);
        // Decrease 1 vehicle on the green quadrant every second
        if (quadrantID == currentGreenRoad && vehicleCounts[quadrantID] > 0) {
            --vehicleCounts[quadrantID];
        }

        // Add 1 vehicle to all red quadrants every 5 seconds
        if (quadrantID != currentGreenRoad && ++redCounter == 5) {
            ++vehicleCounts[quadrantID];
            redCounter = 0;
        }

        // Add 1 vehicle to the green quadrant every 5 seconds
        if (quadrantID == currentGreenRoad && ++greenCounter == 5) {
            ++vehicleCounts[quadrantID];
            greenCounter = 0;
        }
    }
}


void trafficLightControl(int &currentGreenRoad, int runtimeSeconds) {
    auto startTime = chrono::steady_clock::now();
    while (programRunning) {
        auto elapsedTime = chrono::steady_clock::now() - startTime;
        if (chrono::duration_cast<chrono::seconds>(elapsedTime).count() >= runtimeSeconds) {
            programRunning = false;
            break;
        }

        // Green light phase
        displayTrafficLight(currentGreenRoad, false);
        this_thread::sleep_for(chrono::seconds(greenTime));

        // Yellow light phase
        displayTrafficLight(currentGreenRoad, true);
        this_thread::sleep_for(chrono::seconds(yellowTime));

        // Move to the next road
        currentGreenRoad = (currentGreenRoad + 1) % numberOfRoads;
    }
}

int main() {
    int currentGreenRoad;
    int runtimeSeconds;

    // Validate starting quadrant
    currentGreenRoad = getValidatedInput("Select the starting quadrant (1-4): ", 1, 4) - 1;

    // Validate vehicle count for each quadrant
    cout << "Enter the number of vehicles for each quadrant (1-100):\n";
    for (int i = 0; i < numberOfRoads; ++i) {
        vehicleCounts[i] = getValidatedInput("Quadrant " + to_string(i + 1) + ": ", 1, 100);
    }

    // Validate runtime
    runtimeSeconds = getValidatedInput("Enter the total runtime of the program (10-1000 seconds): ", 10, 1000);

    // Start threads
    vector<thread> updaterThreads;
    for (int i = 0; i < numberOfRoads; ++i) {
        updaterThreads.emplace_back(vehicleUpdater, ref(currentGreenRoad), i);
    }

    thread trafficControlThread(trafficLightControl, ref(currentGreenRoad), runtimeSeconds);

    // Join threads
    for (auto &t : updaterThreads) {
        t.join();
    }
    trafficControlThread.join();

    cout << "Program terminated after " << runtimeSeconds << " seconds.\n";
    return 0;
}