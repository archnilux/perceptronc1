#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Perceptron yap�s� (struct)
typedef struct {
    int nIn;        // Girdi say�s�
    int nOut;       // ��kt� say�s�
    int nData;      // Veri say�s�
    double** weights;  // A��rl�k matrisi
} PCN;

// Yard�mc� fonksiyonlar�m�z ��yle olsun
double** allocate_matrix(int rows, int cols) {
	int i;
    double** matrix = (double**)malloc(rows * sizeof(double*));
    for (i = 0; i < rows; i++) {
        matrix[i] = (double*)calloc(cols, sizeof(double));
    }
    return matrix;
}

void free_matrix(double** matrix, int rows) {
	int i;
    for (i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

// Matris �arp�m�: C = A * B
void matrix_multiply(double** A, double** B, double** C, int rowsA, int colsA, int colsB) {
	int i, j, k;
    for (i = 0; i < rowsA; i++) {
        for (j = 0; j < colsB; j++) {
            C[i][j] = 0;
            for (k = 0; k < colsA; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// Perceptron ba�latma
PCN* pcn_init(double** inputs, double** targets, int nData, int nIn, int nOut) {
    PCN* pcn = (PCN*)malloc(sizeof(PCN));
    
    pcn->nIn = nIn;
    pcn->nOut = nOut;
    pcn->nData = nData;
    
    // A��rl�klar� ba�lat (bias dahil)
    pcn->weights = allocate_matrix(nIn + 1, nOut);
    
    // Rastgele a��rl�k atamas� (-0.05 ile 0.05 aras�)
    srand(time(NULL));
    int i, j;
    for (i = 0; i < nIn + 1; i++) {
        for (j = 0; j < nOut; j++) {
            pcn->weights[i][j] = (double)rand() / RAND_MAX * 0.1 - 0.05;
        }
    }
    
    return pcn;
}

// Feedforward
void pcn_forward(PCN* pcn, double** inputs_with_bias, double** activations, int nData) {
    // Matris �arp�m�: activations = inputs * weights
    matrix_multiply(inputs_with_bias, pcn->weights, activations, nData, pcn->nIn + 1, pcn->nOut);
    
    // Aktivasyon fonksiyonu (step function)
    int i, j;
    for (i = 0; i < nData; i++) {
        for (j = 0; j < pcn->nOut; j++) {
            activations[i][j] = (activations[i][j] > 0) ? 1 : 0;
        }
    }
}

// Perceptron e�itimi
void pcn_train(PCN* pcn, double** inputs, double** targets, double eta, int nIterations) {
    // Bias node ekle
    double** inputs_with_bias = allocate_matrix(pcn->nData, pcn->nIn + 1);
    int i, j, iter, k;
    for (i = 0; i < pcn->nData; i++) {
        for (j = 0; j < pcn->nIn; j++) {
            inputs_with_bias[i][j] = inputs[i][j];
        }
        inputs_with_bias[i][pcn->nIn] = -1.0; // Bias
    }
    
    double** activations = allocate_matrix(pcn->nData, pcn->nOut);
    double** errors = allocate_matrix(pcn->nData, pcn->nOut);
    
    for (iter = 0; iter < nIterations; iter++) {
        // Feedforward
        pcn_forward(pcn, inputs_with_bias, activations, pcn->nData);
        
        // Hata hesaplama
        for (i = 0; i < pcn->nData; i++) {
            for (j = 0; j < pcn->nOut; j++) {
                errors[i][j] = activations[i][j] - targets[i][j];
            }
        }
        
        // Weight g�ncelleme
        for (i = 0; i < pcn->nIn + 1; i++) {
            for (j = 0; j < pcn->nOut; j++) {
                double delta = 0;
                for (k = 0; k < pcn->nData; k++) {
                    delta += inputs_with_bias[k][i] * errors[k][j];
                }
                pcn->weights[i][j] -= eta * delta;
            }
        }
        
        printf("Iteration: %d\n", iter);
        printf("Weights:\n");
        for (i = 0; i < pcn->nIn + 1; i++) {
            for (j = 0; j < pcn->nOut; j++) {
                printf("%.4f ", pcn->weights[i][j]);
            }
            printf("\n");
        }
        printf("\n");
    }
    
    // Son ��kt�lar� yazd�r
    pcn_forward(pcn, inputs_with_bias, activations, pcn->nData);
    printf("Final outputs are:\n");
    for (i = 0; i < pcn->nData; i++) {
        for (j = 0; j < pcn->nOut; j++) {
            printf("%.0f ", activations[i][j]);
        }
        printf("\n");
    }
    
    // Belle�i temizle
    free_matrix(inputs_with_bias, pcn->nData);
    free_matrix(activations, pcn->nData);
    free_matrix(errors, pcn->nData);
}

// Confusion Matrix
void pcn_confmat(PCN* pcn, double** inputs, double** targets, int nData) {
    // Bias node ekle
    double** inputs_with_bias = allocate_matrix(nData, pcn->nIn + 1);
    int i, j;
    for (i = 0; i < nData; i++) {
        for (j = 0; j < pcn->nIn; j++) {
            inputs_with_bias[i][j] = inputs[i][j];
        }
        inputs_with_bias[i][pcn->nIn] = -1.0;
    }
    
    double** outputs = allocate_matrix(nData, pcn->nOut);
    matrix_multiply(inputs_with_bias, pcn->weights, outputs, nData, pcn->nIn + 1, pcn->nOut);
    
    int nClasses;
    int* predicted = (int*)calloc(nData, sizeof(int));
    int* actual = (int*)calloc(nData, sizeof(int));
    
    // Binary classification durumu
    if (pcn->nOut == 1) {
        nClasses = 2;
        for (i = 0; i < nData; i++) {
            predicted[i] = (outputs[i][0] > 0) ? 1 : 0;
            actual[i] = (int)targets[i][0];
        }
    } else {
        // Multi-class durumu
        nClasses = pcn->nOut;
        for (i = 0; i < nData; i++) {
            // Argmax bulma
            int maxIdx = 0;
            double maxVal = outputs[i][0];
            for (j = 1; j < pcn->nOut; j++) {
                if (outputs[i][j] > maxVal) {
                    maxVal = outputs[i][j];
                    maxIdx = j;
                }
            }
            predicted[i] = maxIdx;
            
            // Target'ta argmax bulma
            maxIdx = 0;
            maxVal = targets[i][0];
            for (j = 1; j < pcn->nOut; j++) {
                if (targets[i][j] > maxVal) {
                    maxVal = targets[i][j];
                    maxIdx = j;
                }
            }
            actual[i] = maxIdx;
        }
    }
    
    // Confusion matrisi olu�tur
    int** cm = (int**)malloc(nClasses * sizeof(int*));
    for (i = 0; i < nClasses; i++) {
        cm[i] = (int*)calloc(nClasses, sizeof(int));
    }
    
    for (i = 0; i < nData; i++) {
        cm[actual[i]][predicted[i]]++;
    }
    
    // Matrisi yazd�r
    printf("Confusion Matrix:\n");
    for (i = 0; i < nClasses; i++) {
        for (j = 0; j < nClasses; j++) {
            printf("%d ", cm[i][j]);
        }
        printf("\n");
    }
    
    // Accuracy hesapla
    int correct = 0;
    for (i = 0; i < nClasses; i++) {
        correct += cm[i][i];
    }
    printf("Accuracy: %.4f\n", (double)correct / nData);
    
    // Belle�i temizle
    free_matrix(inputs_with_bias, nData);
    free_matrix(outputs, nData);
    free(predicted);
    free(actual);
    for (i = 0; i < nClasses; i++) {
        free(cm[i]);
    }
    free(cm);
}

// PCN'i temizle
void pcn_free(PCN* pcn) {
    free_matrix(pcn->weights, pcn->nIn + 1);
    free(pcn);
}

// �rnek s�n�fland�rma ��yle olabilir;
int main() {
    // �rnek veri seti (AND kap�s�)
    int nData = 4;
    int nIn = 2;
    int nOut = 1;
    
    // Inputs
    double** inputs = allocate_matrix(nData, nIn);
    inputs[0][0] = 0; inputs[0][1] = 0;
    inputs[1][0] = 0; inputs[1][1] = 1;
    inputs[2][0] = 1; inputs[2][1] = 0;
    inputs[3][0] = 1; inputs[3][1] = 1;
    
    // Targets (AND kap�s� ��kt�lar�)
    double** targets = allocate_matrix(nData, nOut);
    targets[0][0] = 0;
    targets[1][0] = 0;
    targets[2][0] = 0;
    targets[3][0] = 1;
    
    // Perceptron'u ba�lat
    PCN* pcn = pcn_init(inputs, targets, nData, nIn, nOut);
    
    // E�it
    printf("Training Perceptron...\n");
    pcn_train(pcn, inputs, targets, 0.25, 10);
    
    // Confusion matrisini g�ster
    printf("\nConfusion Matrix:\n");
    pcn_confmat(pcn, inputs, targets, nData);
    
    // Belle�i temizle
    free_matrix(inputs, nData);
    free_matrix(targets, nData);
    pcn_free(pcn);
    
    return 0;
}
