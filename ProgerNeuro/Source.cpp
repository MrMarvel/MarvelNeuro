#include<fstream>
#include<thread>
#include<random>
#include<time.h>
#include<Windows.h>
#include<iostream>
#include <stdio.h>
#include <conio.h>
#include <map>
#include<vector>

using namespace std;

struct neuron {
	double value;
	double error;
	void act() {
		value = (1 / (1 + pow(2.71828, -value)));
	}
};

struct data_one {
	double info[4096];
	char rresult;
};


class network {
public:
	int layers;
	neuron** neurons;
	double*** weights;
	int* size;
	int threadsNum = 1;/* int(thread::hardware_concurrency());*/
	double sigm_pro(double x) {
		if ((fabs(x - 1) < 1e-9) || (fabs(x) < 1e-9)) return 0.0;
		double res = x * (1.0 - x);
		return res;
	}
	double predict(double x) {
		if (x >= 0.9) {
			return 1;
		}
		else {
			return 0;
		}
	}
	void setLayersNotStudy(int n, int* p, string filename) {
		ifstream fin;
		fin.open(filename);
		srand(time(0));
		layers = n;
		neurons = new neuron * [n];
		weights = new double** [n - 1];
		size = new int[n];
		for (int i = 0; i < n; i++) {
			size[i] = p[i];
			neurons[i] = new neuron[p[i]];
			if (i < n - 1) {
				weights[i] = new double* [p[i]];
				for (int j = 0; j < p[i]; j++) {
					weights[i][j] = new double[p[i + 1]];
					for (int k = 0; k < p[i + 1]; k++) {
						fin >> weights[i][j][k];
					}
				}
			}
		}
	}
	void setLayers(int n, int* p) {
		srand(time(0));
		layers = n;
		neurons = new neuron * [n];
		weights = new double** [n - 1];
		size = new int[n];
		for (int i = 0; i < n; i++) {
			size[i] = p[i];
			neurons[i] = new neuron[p[i]];
			if (i < n - 1) {
				weights[i] = new double* [p[i]];
				for (int j = 0; j < p[i]; j++) {
					weights[i][j] = new double[p[i + 1]];
					for (int k = 0; k < p[i + 1]; k++) {
						weights[i][j][k] = ((rand() % 100)) * 0.01 / size[i];
					}
				}
			}
		}
	}
	void setBestLayers(int n, int* p, string filename) {
		setLayersNotStudy(n, p, filename);
	}
	void setRandomInput() {
		for (int i = 0; i < size[0]; i++) {
			neurons[0][i].value = (rand() % 256) / 255;
		}
	}
	void set_input(double p[]) {
		for (int i = 0; i < size[0]; i++) {
			neurons[0][i].value = p[i];
		}
	}
	void show() {
		setlocale(LC_ALL, "ru");
		cout << "Ядер процессора: " << thread::hardware_concurrency() << endl;
		cout << "Нейронная сеть имеет архитектуру: ";
		for (int i = 0; i < layers; i++) {
			cout << size[i];
			if (i < layers - 1) {
				cout << " - ";
			}
		}cout << endl;
		for (int i = 0; i < layers; i++) {
			cout << "\n#Слой " << i + 1 << "\n\n";
			for (int j = 0; j < size[i]; j++) {
				cout << "Нейрон #" << j + 1 << ": \n";
				cout << "Значение: " << neurons[i][j].value << endl;
				if (i < layers - 1) {
					cout << "Веса: \n";
					for (int k = 0; k < size[i + 1]; k++) {
						cout << "#" << k + 1 << ": ";
						cout << weights[i][j][k] << endl;
					}
				}
			}
		}
	}

	void LayersCleaner(int LayerNumber, int start, int stop) {
		srand(time(0));
		for (int i = start; i < stop; i++) {
			neurons[LayerNumber][i].value = 0;
			//cout << "neurons[" << LayerNumber << "][" << i << "].value = " << neurons[LayerNumber][i].value << endl;
		}
	}

	void ForwardFeeder(int LayerNumber, int start, int stop) {
		for (int j = start; j < stop; j++) {
			for (int k = 0; k < size[LayerNumber - 1]; k++) {
				neurons[LayerNumber][j].value += neurons[LayerNumber - 1][k].value * weights[LayerNumber - 1][k][j];
			}
			//cout << "До активации: " << neurons[i][j].value << endl;
			neurons[LayerNumber][j].act();
		}
	}

	double ForwardFeed() {
		setlocale(LC_ALL, "ru");
		//cout << "Function ForwardFeed:\n";
		//cout << "Threads: " << threadsNum << endl;
		for (int i = 1; i < layers; i++) {
			if (threadsNum == 1) {
				//cout << "Выполняю очистку слоя 1-м ядром...\n";
				thread th1([&]() {
					LayersCleaner(i, 0, size[i]);
					});
				th1.join();
			}
			if (threadsNum == 2) {
				//cout << "Выполняю очистку слоя 2-мя ядрами...\n";
				thread th1([&]() {
					LayersCleaner(i, 0, int(floor(size[i] / 2)));
					});
				thread th2([&]() {
					LayersCleaner(i, int(floor(size[i] / 2)), size[i]);
					});
				th1.join();
				th2.join();
			}
			if (threadsNum == 4) {
				if (size[i] == 1) {
					cout << "Выполняю очистку слоя 1-м ядром...\n";
					thread th1([&]() {
						LayersCleaner(i, 0, size[i]);

						});
					th1.join();
				}
				if ((size[i] == 2) or (size[i] == 3)) {
					cout << "Выполняю очистку слоя 2-мя ядрами...\n";
					thread th1([&]() {
						LayersCleaner(i, 0, int(floor(size[i] / 2)));
						});
					thread th2([&]() {LayersCleaner(i, int(floor(size[i] / 2)), size[i]); });
					th1.join();
					th2.join();
				}
				if (size[i] >= 4) {
					//cout << "Выполняю очистку слоя 4-мя ядрами...\n";
					int start1 = 0;	int stop1 = int(size[i] / 4);
					int start2 = int(size[i] / 4); int stop2 = int(size[i] / 2);
					int start3 = int(size[i] / 2); int stop3 = int(size[i] - floor(size[i] / 4));
					int start4 = int(size[i] - floor(size[i] / 4));	int stop4 = size[i];
					thread th1([&]() {LayersCleaner(i, start1, stop1);  });
					thread th2([&]() {LayersCleaner(i, start2, stop2);  });
					thread th3([&]() {LayersCleaner(i, start3, stop3);  });
					thread th4([&]() {LayersCleaner(i, start4, stop4);  });
					th1.join();
					th2.join();
					th3.join();
					th4.join();
				}
			}
			if (threadsNum == 1) {
				thread th1([&]() {
					ForwardFeeder(i, 0, size[i]);
					});
				th1.join();
			}
			if (threadsNum == 2) {
				thread th1([&]() {
					ForwardFeeder(i, 0, int(floor(size[i] / 2)));
					});
				thread th2([&]() {
					ForwardFeeder(i, int(floor(size[i] / 2)), size[i]);
					});
				th1.join();
				th2.join();
			}
			if (threadsNum == 4) {
				if ((size[i] == 2) or (size[i] == 3)) {
					thread th1([&]() {
						ForwardFeeder(i, 0, int(floor(size[i] / 2)));
						});
					thread th2([&]() {
						ForwardFeeder(i, int(floor(size[i] / 2)), size[i]);
						});
					th1.join();
					th2.join();

				}
				if (size[i] >= 4) {
					int start1 = 0;	int stop1 = int(floor(size[i] / 4));
					int start2 = int(size[i] / 4); int stop2 = int(size[i] / 2);
					int start3 = int(size[i] / 2); int stop3 = int(size[i] - floor(size[i] / 4));
					int start4 = int(size[i] - floor(size[i] / 4));	int stop4 = size[i];
					thread th1([&]() {ForwardFeeder(i, start1, stop1);  });
					thread th2([&]() {ForwardFeeder(i, start2, stop2);  });
					thread th3([&]() {ForwardFeeder(i, start3, stop3);  });
					thread th4([&]() {ForwardFeeder(i, start4, stop4);  });
					th1.join();
					th2.join();
					th3.join();
					th4.join();
				}
			}
		}
		double max = 0;
		double prediction = 0;
		for (int i = 0; i < size[layers - 1]; i++) {
			
			if (neurons[layers - 1][i].value > max) {
				max = neurons[layers - 1][i].value;
				prediction = i;
			}
		}
		return prediction;
	}
	double ForwardFeed(string param) {
		setlocale(LC_ALL, "ru");
		//cout << "Function ForwardFeed:\n";
		//cout << "Threads: " << threadsNum << endl;
		for (int i = 1; i < layers; i++) {
			if (threadsNum == 1) {
				//cout << "Выполняю очистку слоя 1-м ядром...\n";
				thread th1([&]() {
					LayersCleaner(i, 0, size[i]);
					});
				th1.join();
			}
			if (threadsNum == 2) {
				//cout << "Выполняю очистку слоя 2-мя ядрами...\n";
				thread th1([&]() {
					LayersCleaner(i, 0, int(floor(size[i] / 2)));
					});
				thread th2([&]() {
					LayersCleaner(i, int(floor(size[i] / 2)), size[i]);
					});
				th1.join();
				th2.join();
			}
			if (threadsNum == 4) {
				if (size[i] == 1) {
					cout << "Выполняю очистку слоя 1-м ядром...\n";
					thread th1([&]() {
						LayersCleaner(i, 0, size[i]);

						});
					th1.join();
				}
				if ((size[i] == 2) or (size[i] == 3)) {
					cout << "Выполняю очистку слоя 2-мя ядрами...\n";
					thread th1([&]() {
						LayersCleaner(i, 0, int(floor(size[i] / 2)));
						});
					thread th2([&]() {LayersCleaner(i, int(floor(size[i] / 2)), size[i]); });
					th1.join();
					th2.join();
				}
				if (size[i] >= 4) {
					//cout << "Выполняю очистку слоя 4-мя ядрами...\n";
					int start1 = 0;	int stop1 = int(size[i] / 4);
					int start2 = int(size[i] / 4); int stop2 = int(size[i] / 2);
					int start3 = int(size[i] / 2); int stop3 = int(size[i] - floor(size[i] / 4));
					int start4 = int(size[i] - floor(size[i] / 4));	int stop4 = size[i];
					thread th1([&]() {LayersCleaner(i, start1, stop1);  });
					thread th2([&]() {LayersCleaner(i, start2, stop2);  });
					thread th3([&]() {LayersCleaner(i, start3, stop3);  });
					thread th4([&]() {LayersCleaner(i, start4, stop4);  });
					th1.join();
					th2.join();
					th3.join();
					th4.join();
				}
			}
			if (threadsNum == 1) {
				thread th1([&]() {
					ForwardFeeder(i, 0, size[i]);
					});
				th1.join();
			}
			if (threadsNum == 2) {
				thread th1([&]() {
					ForwardFeeder(i, 0, int(floor(size[i] / 2)));
					});
				thread th2([&]() {
					ForwardFeeder(i, int(floor(size[i] / 2)), size[i]);
					});
				th1.join();
				th2.join();
			}
			if (threadsNum == 4) {
				if ((size[i] == 2) or (size[i] == 3)) {
					thread th1([&]() {
						ForwardFeeder(i, 0, int(floor(size[i] / 2)));
						});
					thread th2([&]() {
						ForwardFeeder(i, int(floor(size[i] / 2)), size[i]);
						});
					th1.join();
					th2.join();

				}
				if (size[i] >= 4) {
					int start1 = 0;	int stop1 = int(floor(size[i] / 4));
					int start2 = int(size[i] / 4); int stop2 = int(size[i] / 2);
					int start3 = int(size[i] / 2); int stop3 = int(size[i] - floor(size[i] / 4));
					int start4 = int(size[i] - floor(size[i] / 4));	int stop4 = size[i];
					thread th1([&]() {ForwardFeeder(i, start1, stop1);  });
					thread th2([&]() {ForwardFeeder(i, start2, stop2);  });
					thread th3([&]() {ForwardFeeder(i, start3, stop3);  });
					thread th4([&]() {ForwardFeeder(i, start4, stop4);  });
					th1.join();
					th2.join();
					th3.join();
					th4.join();
				}
			}
		}
		double max = 0;
		double prediction = 0;
		for (int i = 0; i < size[layers - 1]; i++) {
			std::cout << char(i + 65) << " : " << neurons[layers - 1][i].value << endl;
			if (neurons[layers - 1][i].value > max) {
				max = neurons[layers - 1][i].value;
				prediction = i;
			}
		}
		return prediction;
	}

	void ErrorCounter(int LayerNumber, int start, int stop, double prediction, double rresult, double lr) {
		if (LayerNumber == layers - 1) {
			for (int j = start; j < stop; j++) {
				if (j != int(rresult)) {
					neurons[LayerNumber][j].error = -pow((neurons[LayerNumber][j].value),2);
				}
				else {
					neurons[LayerNumber][j].error = pow(1.0 - neurons[LayerNumber][j].value, 2);
				}
			}
		}
		else {
			for (int j = start; j < stop; j++) {
				double error = 0.0;
				for (int k = 0; k < size[LayerNumber + 1]; k++) {
					error += neurons[LayerNumber + 1][k].error * weights[LayerNumber][j][k];
				}
				neurons[LayerNumber][j].error = error;
			}
		}

	}

	void WeightsUpdater(int start, int stop, int LayerNum, int lr) {
		int i = LayerNum;
		for (int j = start; j < stop; j++) {
			for (int k = 0; k < size[i + 1]; k++) {
				weights[i][j][k] += lr * neurons[i + 1][k].error * sigm_pro(neurons[i + 1][k].value) * neurons[i][j].value;
			}
		}
	}

	void BackPropogation(double prediction, double rresult, double lr) {
		for (int i = layers - 1; i > 0; i--) {
			if (threadsNum == 1) {
				if (i == layers - 1) {
					for (int j = 0; j < size[i]; j++) {
						if (j != int(rresult)) {
							neurons[i][j].error = -pow((neurons[i][j].value), 2);
						}
						else {
							neurons[i][j].error = pow(1.0 - neurons[i][j].value, 2);
						}

					}
				}
				else {
					for (int j = 0; j < size[i]; j++) {
						double error = 0.0;
						for (int k = 0; k < size[i + 1]; k++) {
							error += neurons[i + 1][k].error * weights[i][j][k];
						}
						neurons[i][j].error = error;
					}
				}
			}
			if (threadsNum == 2) {
				thread th1([&]() {
					ErrorCounter(i, 0, int(size[i] / 2), prediction, rresult, lr);
					});
				thread th2([&]() {
					ErrorCounter(i, int(size[i] / 2), size[i], prediction, rresult, lr);
					});
				th1.join();
				th2.join();
			}
			if (threadsNum == 4) {
				if (size[i] < 4) {
					if (i == layers - 1) {
						for (int j = 0; j < size[i]; j++) {
							if (j != int(rresult)) {
								neurons[i][j].error = -(neurons[i][j].value);
							}
							else {
								neurons[i][j].error = 1.0 - neurons[i][j].value;
							}

						}
					}
					else {
						for (int j = 0; j < size[i]; j++) {
							double error = 0.0;
							for (int k = 0; k < size[i + 1]; k++) {
								error += neurons[i + 1][k].error * weights[i][j][k];
							}
							neurons[i][j].error = error;
						}
					}
				}
				if (size[i] >= 4) {
					int start1 = 0;	int stop1 = int(size[i] / 4);
					int start2 = int(size[i] / 4); int stop2 = int(size[i] / 2);
					int start3 = int(size[i] / 2); int stop3 = int(size[i] - floor(size[i] / 4));
					int start4 = int(size[i] - floor(size[i] / 4));	int stop4 = size[i];
					thread th1([&]() {
						ErrorCounter(i, start1, stop1, prediction, rresult, lr);
						});
					thread th2([&]() {
						ErrorCounter(i, start2, stop2, prediction, rresult, lr);
						});
					thread th3([&]() {
						ErrorCounter(i, start3, stop3, prediction, rresult, lr);
						});
					thread th4([&]() {
						ErrorCounter(i, start4, stop4, prediction, rresult, lr);
						});
					th1.join();
					th2.join();
					th3.join();
					th4.join();

				}
			}
		}
		for (int i = 0; i < layers - 1; i++) {
			if (threadsNum == 1) {
				for (int j = 0; j < size[i]; j++) {
					for (int k = 0; k < size[i + 1]; k++) {
						weights[i][j][k] += lr * neurons[i + 1][k].error * sigm_pro(neurons[i + 1][k].value) * neurons[i][j].value;
					}
				}
			}
			if (threadsNum == 2) {
				thread th1([&]() {
					WeightsUpdater(0, int(size[i] / 2), i, lr);
					});
				thread th2([&]() {
					WeightsUpdater(int(size[i] / 2), size[i], i, lr);
					});
				th1.join();
				th2.join();
			}
			if (threadsNum == 4) {
				if (size[i] < 4) {
					for (int j = 0; j < size[i]; j++) {
						for (int k = 0; k < size[i + 1]; k++) {
							weights[i][j][k] += lr * neurons[i + 1][k].error * sigm_pro(neurons[i + 1][k].value) * neurons[i][j].value;
						}
					}
				}
				if (size[i] >= 4) {
					int start1 = 0;	int stop1 = int(size[i] / 4);
					int start2 = int(size[i] / 4); int stop2 = int(size[i] / 2);
					int start3 = int(size[i] / 2); int stop3 = int(size[i] - floor(size[i] / 4));
					int start4 = int(size[i] - floor(size[i] / 4));	int stop4 = size[i];
					thread th1([&]() {
						WeightsUpdater(start1, stop1, i, lr);
						});
					thread th2([&]() {
						WeightsUpdater(start2, stop2, i, lr);
						});
					thread th3([&]() {
						WeightsUpdater(start3, stop3, i, lr);
						});
					thread th4([&]() {
						WeightsUpdater(start4, stop4, i, lr);
						});
					th1.join();
					th2.join();
					th3.join();
					th4.join();

				}
			}
		}
	}

	bool SaveWeights() {
		ofstream fout;
		fout.open("weights.txt");
		for (int i = 0; i < layers; i++) {
			if (i < layers - 1) {
				for (int j = 0; j < size[i]; j++) {
					for (int k = 0; k < size[i + 1]; k++) {
						fout << weights[i][j][k] << " ";
					}
				}
			}
		}
		fout.close();
		return 1;
	}
};

bool operator<(const data_one& d1, const data_one& d2) {
	return (d1.rresult < d2.rresult);
}


int main() {
	//system("start test_creator.pas");
	srand(time(0));
	setlocale(LC_ALL, "Russian");
	ifstream fin;
	ofstream fout;
	fout.open("log.txt");
	const int l = 4;
	const int input_l = 4096;
	int size[l] = { input_l, 256, 64,   26 };
	network nn;

	double input[input_l];
	//double rresult;
	char rresult;
	const int maxDx = 3;
	const int maxDy = 2;
	double result;
	double ra = 0;
	int maxra = 0;
	int last_maxraepoch = 0;
	int maxraepoch = 0;
	int n = 83;
	int to_study = 0;
	std::cout << "Производить обучение? [-1542] - начать с нуля [1] - продолжить [0] - нет" << endl;
	std::cin >> to_study;
	double time = 0;

	vector<data_one> libData;
	vector<data_one> testLibData;
	vector<data_one> allData;
	vector<pair<data_one,vector<int>>> shiftedAllData;
	libData.resize(n);

	if (to_study == 12 || to_study == 1 || to_study == -1542) {
		if (to_study == 12 || to_study -1542){

			fin.open("lib.txt");
			for (int i = 0; i < n; i++) {
				for (int j = 0; j < input_l; j++) {
					fin >> libData[i].info[j];
				}
				fin >> libData[i].rresult;
				libData[i].rresult -= 65;
			}
		}
		fin.close();
		double ra = 0;
		int maxra = 0;
		int maxraepoch = 0;
		cout << "Теперь обучаем на testlib.txt" << endl;
		{
			fin.open("testlib.txt");
			for (int i = 0; !fin.eof() && fin.good(); i++) {
				try {
					testLibData.resize(testLibData.size() + 1);
					for (int j = 0; j < input_l; j++) {
						fin >> testLibData[i].info[j];
					}
					fin >> testLibData[i].rresult;
					testLibData[i].rresult -= 65;
					if (fin.eof() || !fin.good()) throw 1;
				} catch (int except) {
					testLibData.resize(testLibData.size() - 1);
				}
			}
			allData.insert(allData.end(), libData.begin(), libData.end());
			allData.insert(allData.end(), testLibData.begin(), testLibData.end());
			for (int i = 0; i < allData.size(); i++) {
				for (int dx = -maxDx; dx <= maxDx; dx++) {
					for (int dy = -maxDy; dy <= maxDy; dy++) {
						shiftedAllData.resize(shiftedAllData.size() + 1);
						data_one shiftedData;
						for (int y = 0; y < sqrt(input_l); y++) {
							for (int x = 0; x < sqrt(input_l); x++) {
								int shiftedDataJ = y * sqrt(input_l) + x;
								int dataJ = (y+dy) * sqrt(input_l) + (x+dx);
								if (0 <= x + dx && x + dx < sqrt(input_l) && 0 <= y + dy && y + dy < sqrt(input_l)) {
									shiftedData.info[shiftedDataJ] = allData[i].info[dataJ];
								} else {
									shiftedData.info[shiftedDataJ] = 0;
								}
							}
						}
						shiftedData.rresult = allData[i].rresult;
						vector<int> shifting = {dx,dy};
						shiftedAllData[shiftedAllData.size()-1] = pair<data_one,vector<int>>(shiftedData, shifting);
					}
				}
			}
			for (int i = 0; i < shiftedAllData.size(); i++) {
				for (int j = 0; j < input_l; j++) {
					if (j % 64 == 0 && j != 0) {
						//cout << endl;
					}
					//cout << (int) shiftedAllData[i].first.info[j];
				}
				//cout << endl;
				//cout << endl;
			}
			n = shiftedAllData.size();
			if (to_study == -1542) {
				nn.setLayers(l, size);
			} else {
				nn.setBestLayers(l, size, "weights.txt");
			}
			for (int e = 0; ra / n * 100 < 100; e++) {
				//cout << "Epoch #" << e << endl;
				fout << "Epoch # " << e << endl;
				double epoch_start = clock();
				ra = 0;
				double w_delta = 0;


				const int countOfSymbols = 26;
				unsigned int countOfSamplesOnSymbol[countOfSymbols] = {};
				for (int i = 0; i < n; i++) {
					data_one data = shiftedAllData[i].first;
					int dx = shiftedAllData[i].second[0];
					int dy = shiftedAllData[i].second[1];

					for (int j = 0; j < 4096; j++) {
						input[j] = data.info[j];
						if (j % 64 == 0 && j != 0) {
							//cout << endl;
						}
						//cout << (int)input[j];
					}
					//cout << endl;
					rresult = data.rresult;
					//cout << int(rresult) << endl;
					//cout << "Цифра " << rresult << endl;
					nn.set_input(input);
					double FF_start = clock();

					result = nn.ForwardFeed();

					double FF_stop = clock();
					//cout << "ForwardFeed Time: " << FF_stop - FF_start << endl;
					//nn.show();
					if (dx == -maxDx && dy == -maxDy) countOfSamplesOnSymbol[rresult]++;
					if (result == rresult) {
						//cout << "Результат верный!\n";
						/*cout << "Угадал букву " << char(rresult + 65);
						if (countOfSamplesOnSymbol[rresult] != 1) cout << countOfSamplesOnSymbol[rresult];
						if (dx != 0 || dy != 0) cout << " смещение";
						if (dx != 0) cout << " dx:" << dx;
						if (dy != 0) cout << " dy:" << dy;
						cout << "\t\t\t****" << endl;*/
						ra++;
					} else {
						//cout << "Результат " << result << " неверный!\n";
						//cout << "Не угадал букву " << char(rresult + 65) << "\n";
						double BP_start = clock();
						nn.BackPropogation(result, rresult, 0.5);
						double BP_stop = clock();
						//cout << "BackPropogation time: " << BP_stop - BP_start << endl;
						//cout << endl;
					}
				}

				double epoch_stop = clock();
				cout << "Right answers: " << ra / n * 100 << "% \t Max RA: " << double(maxra) / n * 100 << "(epoch " << maxraepoch << " )" << endl;
				time = 0; cout << "";
				//cout << "W_Delta: " << w_delta << endl;
				if (ra > maxra) {
					maxra = ra;
					maxraepoch = e;

					if (maxraepoch - last_maxraepoch >= 1) {
						if (maxraepoch - last_maxraepoch >= 5) {
							if (nn.SaveWeights()) {
								cout << "Веса сохранены!" << endl;
							}
							last_maxraepoch = maxraepoch;
						} 
					} else if (e - maxraepoch >= 5) {
						if (nn.SaveWeights()) {
							cout << "Веса сохранены!" << endl;
						}
					}
				}
				if (maxraepoch < e - 250) {
					maxra = 0;
				}
			}
			//fout.close();
			if (nn.SaveWeights()) {
				cout << "Веса сохранены!" << endl;
			}
		}
	} else {
		nn.setLayersNotStudy(l, size, "weights.txt");
	}
	fin.close();

	cout << "Начать тест:(1/0) ";
	bool to_start_test = 0;
	cin >> to_start_test;
	char right_res;
	if (to_start_test) {
		fin.open("test.txt");
		for (int i = 0; i < input_l; i++) {
			fin >> input[i];
		}
		nn.set_input(input);
		result = nn.ForwardFeed(string("show results"));
		cout << "Я считаю, что это буква " << char(result + 65) << "\n\n";
		cout << "А какая это буква на самом деле?...";
		cin >> right_res;
		if (right_res != result + 65) {
			cout << "Хорошо господин, исправляю ошибку!";
			nn.BackPropogation(result, right_res - 65, 0.15);
			nn.SaveWeights();
		}
	}
	cout << "Начать универсальный тест: ";
	bool start_universal = 0;
	cin >> start_universal;
	

	if (start_universal) {
		fin.close();
		int length;
		int rightAnswers = 0;
		cout << "Введите кол-во примеров в тесте: ";
		cin >> length;
		fin.open("universal_test.txt");
		for (int i = 0; i < length; i++) {
			for (int j = 0; j < input_l; j++) {
				fin >> input[j];
			}
			fin >> right_res;
			nn.set_input(input);
			result = nn.ForwardFeed();
			if (right_res == result + 65) {
				rightAnswers++;
			} else {
				cout << "Ошибся с буквой " << right_res << ". Вывело " << char(result+65) << endl;
			}
		}
		cout << "Тест пройден! Процент правильных ответов: " << double(rightAnswers) / double(length) * 100 << " %\n";
		system("pause");
	}


	return 0;
}