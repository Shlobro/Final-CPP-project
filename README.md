SafetyNet
SafetyNet is a real-time alert system for monitoring live and historical alerts from www.oref.org.il. The application continuously fetches live alerts, parses the data, and displays it through a graphical user interface (GUI). Users can view live alerts, historical data, and additional information.

Table of Contents
Features
Installation
Usage
Configuration
Dependencies
Contributing
License
Features
Live Alerts: Fetch and display real-time alerts from a specified URL.
Alert History: Retrieve and view historical alert data.
GUI with ImGui: A user-friendly graphical interface for monitoring alerts.
Search Functionality: Search for alerts by location.
Fade-In Effect: Smooth appearance of titles and subtitles.
Installation
Prerequisites
Ensure you have the following software installed:

C++17 or later
CMake
A suitable C++ compiler (e.g., GCC, Clang, MSVC)
OpenSSL for secure HTTP connections
Steps
Clone the Repository:

sh
Copy code
git clone <repository-url>
cd SafetyNet
Build the Project:

sh
Copy code
mkdir build
cd build
cmake ..
make
Run the Application:

sh
Copy code
./SafetyNet
Usage
Launch the Application: Run the executable after building the project.
View Live Alerts: The main screen displays live alerts. Use the search bar to filter alerts by location.
Alert History: Click the "Alert's History" button to view historical alerts.
Instructions: Access detailed instructions by clicking the "Instructions" button.
Exit: Click the "Exit" button or use the ESC key to close the application.
Configuration
Live Alerts URL: Set the live alerts URL in the DownloadThread::setLiveLink function.
History URL: Set the history URL in the DownloadThread::setHistoryLink function.
Fade-In Effect: Customize the fade-in speed by adjusting alpha_increment in the DrawThread class.
Dependencies
CPP-HTTPLIB - HTTP library for C++
nlohmann/json - JSON parser for C++
OpenSSL - Toolkit for secure communications
Contributing
We welcome contributions! Please fork the repository and submit a pull request with your changes. Ensure that your code follows the project's style and includes necessary documentation.

License
This project is licensed under the MIT License. See the LICENSE file for details.
