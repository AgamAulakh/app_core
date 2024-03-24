#include "ArmMatrixWrapper.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "FFTsignal.h"

using namespace std;

#define LISA_TEST_PROJECT 1
#define RAW_SAMPLE_NUMBER 1024 // Just testing for one channel
#define PROCESSED_SAMPLE_NUMBER (RAW_SAMPLE_NUMBER/2)
#define SAMPLE_FREQ 250 
#define CHANNELS 1 // Just for testing one channel 
#define BANDS 4

 enum PowerBands : uint8_t {
        DELTA = 0,
        THETA,
        ALPHA,
        BETA,
};

int main () {
    // Read the CSV file 
    ifstream file("Test_1.csv");
    
    if (!file.is_open()) {
        cerr << "Error opening file!" << endl;
        return 1;
    }
    
    string line;
    vector< vector<string> > rawData; // Vector of vectors to store CSV rawData
    
    // Read each line from the file
    while (getline(file, line)) {
        stringstream ss(line);
        string item;
        vector<string> items;
        
        // Split the line by commas and store in a vector
        while (getline(ss, item, ',')) {
            items.push_back(item);
        }
        
        // Add the vector of items to the rawData vector
        rawData.push_back(items);
    }
    
    // Close the file
    file.close();
    
    // 256 samples, with 8 channels
    ArmMatrixWrapper<RAW_SAMPLE_NUMBER, 8> allChannels;

    // Array of FFT results of all 8 channels
    vector< ArmMatrixWrapper<PROCESSED_SAMPLE_NUMBER, 1> > channelFFTResults(CHANNELS);
  
    // Array of Power spectrum of all 8 channels 
    vector< ArmMatrixWrapper<PROCESSED_SAMPLE_NUMBER, 1> > channelPowerResults(CHANNELS);
   
    // Array of channels where each channel has 4 elements for 4 bandpowers
    vector<vector<float32_t>> channelBandPowers(CHANNELS, std::vector<float32_t>(BANDS));

    // Array of channels where each channel has 4 elements for 4 bandpowers
    vector<vector<float32_t>> channelRelativeBandPowers(CHANNELS, std::vector<float32_t>(BANDS));
 
    // Fill the matrix channels with values
    // for (size_t row = 0; row < rawData.size(); row++) {
    //     for (size_t col = 1; col < rawData[row].size(); col++) {
    //         allChannels.set_at(stof(rawData[row][col]), row, col);
    //        std::cout << "Row: " << row << ", Column: " << col << ", Value: " << rawData[row][col] << std::endl;

    //     }
    // }

    for (size_t i = 0; i < RAW_SAMPLE_NUMBER; i++) {
        allChannels.set_at(inputSignal[i], i, 1);
        std::cout << "Row: " << i << ", Column: " << 1 << ", Value: " << inputSignal[i] << std::endl; 
    }
    
    // Note: Loop unrolling to help optimize and parallelize code
    // Preprocessing steps 
    for (size_t i = 0; i < CHANNELS; i=i+2) 
    {
         // Compute the singeSideFFT for all channels
        channelFFTResults[i] = allChannels.singleSideFFT(i);
        std::cout << "Print single-sided FFT results" <<  std::endl; 
        channelFFTResults[i].prettyPrint();
       // channelFFTResults[i+1] = allChannels.singleSideFFT(i+1);

        // Compute the singleSidePower for all channels
        channelPowerResults[i] = allChannels.singleSidePower(i);
         std::cout << "Print single-sided  results" <<  std::endl; 
        channelPowerResults[i].prettyPrint();
        //channelPowerResults[i+1] = allChannels.singleSidePower(i+1);

    }

    // Computation of bandpowers
    for (size_t i = 0; i < CHANNELS; i=i+2) 
    {
        // Bandpower for delta 
        channelBandPowers[i][0] = channelPowerResults[i].singleSideBandPower(i, SAMPLE_FREQ, RAW_SAMPLE_NUMBER, 0);
       // channelBandPowers[i+1][0] = channelPowerResults[i+1].singleSideBandPower(i+1, SAMPLE_FREQ, RAW_SAMPLE_NUMBER, 0);
       
        // Bandpower for theta 
        channelBandPowers[i][1] = channelPowerResults[i].singleSideBandPower(i, SAMPLE_FREQ, RAW_SAMPLE_NUMBER, 1);
       // channelBandPowers[i+1][1] = channelPowerResults[i+1].singleSideBandPower(i+1, SAMPLE_FREQ, RAW_SAMPLE_NUMBER, 1);

        // Bandpower for alpha 
        channelBandPowers[i][2] = channelPowerResults[i].singleSideBandPower(i, SAMPLE_FREQ, RAW_SAMPLE_NUMBER, 2);
       // channelBandPowers[i+1][2] = channelPowerResults[i+1].singleSideBandPower(i+1, SAMPLE_FREQ, RAW_SAMPLE_NUMBER, 2);

        // Bandpower for beta 
        channelBandPowers[i][3] = channelPowerResults[i].singleSideBandPower(i, SAMPLE_FREQ, RAW_SAMPLE_NUMBER, 3);
       // channelBandPowers[i+1][3] = channelPowerResults[i+1].singleSideBandPower(i+1, SAMPLE_FREQ, RAW_SAMPLE_NUMBER, 3);

    }

    // Computation of relative bandpowers
    for (size_t i = 0; i < CHANNELS; i=i+2) 
    {
        // Each channel's power spectrum calculates the relative band powers of the 4 bands and stores them into 
        // a 2D array. Outer index denotes the channel number, inner index denotes the band power value
        channelRelativeBandPowers[i] = channelPowerResults[i].singleSideRelativeBandPower(channelBandPowers[i]);
        //channelRelativeBandPowers[i+1] = channelPowerResults[i+1].singleSideRelativeBandPower(channelBandPowers[i+1]);

    }


    return 0;

}