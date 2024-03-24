// #include <stdint.h>
#include <cstdint>
#include <cstring>
#include <vector>
#include <zephyr/sys/printk.h> 


// #include "arm_math/DSP/Include/arm_math.h"
// #include "v1.9.1/modules/hal/cmsis/CMSIS/DSP/Include/arm_math.h"

//using namespace std;
//#include "v1.9.1/modules/hal/cmsis/CMSIS/Core/Include/cmsis_compiler.h"
//#include "../../lisa_test_project/arm_math/DSP/include/arm_math.h"
// #if defined(LISA_TEST_PROJECT)
// #include "../../lisa_test_project/arm_math/arm_math.h"
// #else
//#include "../../lisa_test_project/arm_math/DSP/Include/arm_math.h"
// #endif
#include <arm_math.h>

template <uint32_t MaxRows, uint32_t MaxCols>
class ArmMatrixWrapper {
private:
    arm_matrix_instance_f32 matrix;

public:
    static constexpr uint32_t MaxBufferSize = MaxRows * MaxCols;
    float32_t data[MaxBufferSize];
    
    ArmMatrixWrapper() {
        arm_mat_init_f32(&matrix, MaxRows, MaxCols, data);
    };

    // Copy cstor
    ArmMatrixWrapper(const ArmMatrixWrapper& other) {
        memcpy(data, other.data, MaxBufferSize * sizeof(float32_t));
        arm_mat_init_f32(&matrix, MaxRows, MaxCols, data);
    };

    // Assignment optor
    ArmMatrixWrapper& operator=(const ArmMatrixWrapper& other) {
        if (this != &other) {
            memcpy(data, other.data, MaxBufferSize * sizeof(float32_t));
            arm_mat_init_f32(&matrix, MaxRows, MaxCols, data);
        }
        return *this;
    };

    // Dstor
    ~ArmMatrixWrapper() {};

    // Getters and Setters
    bool set_rows(uint32_t rows) {
        if (rows <= MaxRows) {
            arm_mat_init_f32(&matrix, rows, matrix.numCols, data);
            return true;
        } else {
            // NOTE: silent failure
            return false;
        }
    };

    bool set_cols(uint32_t cols) {
        if (cols <= MaxCols) {
            arm_mat_init_f32(&matrix, matrix.numRows, cols, data);
            return true;
        } else {
            // Handle error: Columns exceeds maximum allowed value
            return false;
        }
    };

    uint32_t get_rows() const {
        return matrix.numRows;
    };

    uint32_t get_cols() const {
        return matrix.numCols;
    };


    // Access Functions
    float32_t at(uint32_t index) const {
        if (index < MaxBufferSize) {
            return data[index];
        } else {
            // Note: silent error
            return 0.0f;
        }
    };

    bool set_at(float32_t value, uint32_t index) {
        if (index < MaxBufferSize) {
            data[index] = value;
            return true;
        } else {
            return false;
        }
    };

    float32_t at(uint32_t i, uint32_t j) const {
        if (i < matrix.numRows && j < matrix.numCols) {
            return data[i * matrix.numCols + j];
        } else {
            return 0.0f;
        }
    };

    void prettyPrint() {

        printk("\nmatrix data:\n") ;
        for (int row = 0; row < matrix.numRows; row++ ) { 
            for (int col = 0; col < matrix.numCols; col++ ) 
            { 
                printk("%.4f\t", this->at(row,col));
            } 
        } 
        printk("\n");

    }

    bool set_at(float32_t value, uint32_t i, uint32_t j) {
        if (i < matrix.numRows && j < matrix.numCols) {
            data[i * matrix.numCols + j] = value;
            return true;
        } else {
            return false;
        }
    };

    // Overloaded Operators
    ArmMatrixWrapper operator+(const ArmMatrixWrapper& other) const {
        ArmMatrixWrapper result; // deepy copy
        arm_mat_add_f32(&matrix, &other.matrix, &result.matrix);
        return result;
    };

    ArmMatrixWrapper operator-(const ArmMatrixWrapper& other) const {
        ArmMatrixWrapper result;
        arm_mat_sub_f32(&matrix, &other.matrix, &result.matrix);
        return result;
    };

    ArmMatrixWrapper operator*(const ArmMatrixWrapper& other) const {
        ArmMatrixWrapper result;
        arm_mat_mult_f32(&matrix, &other.matrix, &result.matrix);
        return result;
    };

    ArmMatrixWrapper& operator+=(const ArmMatrixWrapper& other) {
        arm_mat_add_f32(&matrix, &other.matrix, &matrix);
        return *this;
    };

    ArmMatrixWrapper& operator-=(const ArmMatrixWrapper& other) {
        arm_mat_sub_f32(&matrix, &other.matrix, &matrix);
        return *this;
    };

    ArmMatrixWrapper& operator*=(const ArmMatrixWrapper& other) {
        arm_mat_mult_f32(&matrix, &other.matrix, &matrix);
        return *this;
    };

    // Matrix Operations
    ArmMatrixWrapper<MaxCols, MaxRows> transpose() const {
        ArmMatrixWrapper<MaxCols, MaxRows> result;
        arm_mat_trans_f32(&matrix, &result.matrix);
        return result;
    };

    bool inverse() {
        arm_status status = arm_mat_inverse_f32(&matrix, &matrix);
        return (status == ARM_MATH_SUCCESS);
    };

    bool scale(float32_t scaleFactor) {
        arm_mat_scale_f32(&matrix, scaleFactor, &matrix);
        return true; // Assuming the CMSIS-DSP function always succeeds
    };

    // Utility
    void set_to_identity() {
        // arm_mat_identity_f32(&matrix);
    };

    void set_to_zero() {
        memset(data, 0, MaxBufferSize * sizeof(float32_t));
        matrix.numCols = 0;
        matrix.numRows = 0;
    };

    // statistics
    float32_t maximum() const {
        float32_t maxValue;
        uint32_t maxIndex;
        arm_max_f32(data, MaxBufferSize, &maxValue, &maxIndex);
        return maxValue;
    };

    float32_t minimum() const {
        float32_t minValue;
        uint32_t minIndex;
        arm_min_f32(data, MaxBufferSize, &minValue, &minIndex);
        return minValue;
    };

    float32_t mean() const {
        float32_t meanValue;
        arm_mean_f32(data, MaxBufferSize, &meanValue);
        return meanValue;
    };

    float32_t power() const {
        float32_t powerValue;
        arm_power_f32(data, MaxBufferSize, &powerValue);
        return powerValue;
    };

    float32_t rms() const {
        float32_t rmsValue;
        arm_rms_f32(data, MaxBufferSize, &rmsValue);
        return rmsValue;
    };

    float32_t standardDeviation() const {
        float32_t stdDevValue;
        arm_std_f32(data, MaxBufferSize, &stdDevValue);
        return stdDevValue;
    };

    float32_t variance() const {
        float32_t varianceValue;
        arm_var_f32(data, MaxBufferSize, &varianceValue);
        return varianceValue;
    };

    float32_t meanSquareError(const ArmMatrixWrapper<MaxRows, MaxCols>& other) const {
        float32_t mseValue;
        arm_mean_squared_error_f32(data, other.data, MaxBufferSize, &mseValue);
        return mseValue;
    };

    // Computes the raw FFT for specific channel given a number of samples 
    ArmMatrixWrapper<MaxRows/2, 1> rawFFT(uint32_t channel) const {
        
        // Struct to store the output of the FFT for one channel 
        ArmMatrixWrapper<MaxRows/2, 1> rawResult;

        // Temporary arrays to store initial data and intermediate FFT values
        float32_t inputFFT[matrix.numRows];
        float32_t outputFFT[matrix.numRows];
       
        // O indicates FFT and 1 indicates inverse FFT (This flag will not change)
        const uint32_t ifftFlag = 0;

        // Initalize the instance with the specified number of rows
        arm_rfft_fast_instance_f32 fft_instance;
        arm_rfft_fast_init_f32(&fft_instance, matrix.numRows);
        
        for (uint32_t i = 0; i < matrix.numRows; i++)
        {
            inputFFT[i] = at(i, channel);
        }

        // Compute the FFT and store the outputs of the FFT in the array
        arm_rfft_fast_f32(&fft_instance, inputFFT , outputFFT, ifftFlag);

        // The first entry is the DC offset so simply set to 0
        outputFFT[0] =  0;

        // Copy output FFT to result
        memcpy(rawResult.data, outputFFT, MaxBufferSize * sizeof(float32_t));
        return rawResult;
    };


    // Computes the single-sided FFT for a specific channel given a number of samples
    ArmMatrixWrapper<MaxRows/2, 1> singleSideFFT(uint32_t channel) const {

        // Struct to store the output of the FFT for one channel 
        ArmMatrixWrapper<MaxRows/2, 1> FFTResult;
        
        // Temporary arrays to store initial data and intermediate FFT values
        float32_t outputFFTMag[matrix.numRows/2];

        // Extract the magnitude values because FFT values are complex and have both a magnitude and phase.
        // matrix.numRows/2 because we are looking at one-sided spectrum (# of magnitudes we want to look at)
        arm_cmplx_mag_f32(rawFFT(channel).data, outputFFTMag, matrix.numRows/2);

        // Copy the output to the result matrix
        memcpy(FFTResult.data, outputFFTMag, MaxBufferSize * sizeof(float32_t));

        return FFTResult;

    }

    // Computes the single-sided Power given the single-sided FFT of a channel
    ArmMatrixWrapper<MaxRows/2, 1> singleSidePower(uint32_t channel) const {
        
        // Struct to store the output of the FFT for one channel 
        ArmMatrixWrapper<MaxRows/2, 1> PowerResult;
        
        // Temporary arrays to store power FFT values
        float32_t powerFFT[matrix.numRows/2];

        // Compute the power of the FFT
        arm_cmplx_mag_squared_f32(rawFFT(channel).data, powerFFT, matrix.numRows/2);
        
        // Check if the scaling is required for the above function
        // Copy the output to the result matrix
        memcpy(PowerResult.data, powerFFT, MaxBufferSize * sizeof(float32_t));
        return PowerResult;

    }

    // Computes the single-sided Band Power given the single-sided FFT of a channel,
    // Pwelch of specified channel and specific band power range type
    float32_t singleSideBandPower(uint32_t channel, uint32_t sampleFreq, uint32_t sampleNo, uint32_t bandSelect) const {
        
        // The frequency resolution or frequency bin width
        float32_t freqRes = sampleFreq / sampleNo;

        // The following band limits: delta band, theta band, alpha band, beta band
        float32_t bandRanges[] = {1,3,4,7,8,12,13,30};

        // Low and high limit for specified band range of frequencies
        uint32_t lowLimit = 0;
        uint32_t highLimit = 0;

        switch(bandSelect){

            // delta band (Note: make an enum for the different band types)
            case 0:{
                lowLimit = (uint32_t)(floor(bandRanges[0] / freqRes));  
                highLimit = (uint32_t)(ceil(bandRanges[1] / freqRes));
                break;
            } 
            // theta band
            case 1:{
                lowLimit = (uint32_t)(floor(bandRanges[2] / freqRes));
                highLimit = (uint32_t)(ceil(bandRanges[3] / freqRes));
                break;
            }
            // alpha band
            case 2:{
                lowLimit = (uint32_t)(floor(bandRanges[4] / freqRes));
                highLimit = (uint32_t)(ceil(bandRanges[5] / freqRes));
                break;
            }
            // beta band
            case 3:{
                lowLimit = (uint32_t)(floor(bandRanges[6] / freqRes));
                highLimit = (uint32_t)(ceil(bandRanges[7] / freqRes));
                break;
            }
            default:{
                break;
            }

        }

        // Calculating the band power for specified band range of frequnencies
        float32_t bandPower = 0;
        for(uint32_t i = lowLimit; i < highLimit; lowLimit++){
            bandPower += (data[i] * freqRes);
        }
        return bandPower;
    }

    // Computes the single-sided Relative Band Power given the specified bandPower values
   std::vector<float32_t> singleSideRelativeBandPower(std::vector<float32_t> bandPowers){
        
        std::vector<float32_t> relativeBandpowers(4);

        // Calculate summation of total power for one channel
        float32_t totalPower = 0;
        for(uint32_t i = 0; i < MaxBufferSize; i++){
            totalPower += data[i];
        }

        // Calculate relative band power relative to the total power of one channel
        for(uint32_t i = 0; i < bandPowers.size(); i++){

            relativeBandpowers[i] = bandPowers[i] / totalPower;
        }

        return relativeBandpowers;
    }

};