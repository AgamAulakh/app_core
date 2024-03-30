#include <stdint.h>
#include <cstring>
#include "arm_math.h"

template <uint32_t MaxRows, uint32_t MaxCols>
class ArmMatrixWrapper {
private:
    static constexpr uint32_t MaxBufferSize = MaxRows * MaxCols;
    float32_t data[MaxBufferSize];
    arm_matrix_instance_f32 matrix;

public:
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

    bool set_at(float32_t value, uint32_t i, uint32_t j) {
        if (i < matrix.numRows && j < matrix.numCols) {
            data[i * matrix.numCols + j] = value;
            return true;
        } else {
            return false;
        }
    };

    void pretty_print() {
        printk("\nmatrix data:\n");
        for (int row = 0; row < matrix.numRows; row++ ) { 
            for (int col = 0; col < matrix.numCols; col++) { 
                printk("%.4f\t", this->at(row,col));
            } 
        } 
        printk("\n");
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

    // // TEMPORARY!!!
    // // Fast Fourier Transform Wrapper (FFT)
    // // Assumes input is real data row vector, and output will be complex row vector
    // ArmMatrixWrapper<1, MaxCols> fft() const {
    //     ArmMatrixWrapper<1, MaxCols> result;
    //     arm_cfft_f32(&arm_cfft_sR_f32_len, reinterpret_cast<float32_t*>(data), 0, 1);
    //     memcpy(result.data, data, MaxBufferSize * sizeof(float32_t));
    //     return result;
    // };
};