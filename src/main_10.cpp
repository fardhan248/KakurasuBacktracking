#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <deque>
#include <unordered_set>
#include <chrono>

using namespace std;
using namespace chrono;

vector<string> split(const string& str, char delim) {
    vector<string> splitted;
    istringstream ss(str);
    string token;
    while (getline(ss, token, delim)) {
        splitted.push_back(token);
    }
    return splitted;
}

void PrintBoardChar(vector<vector<string>>& board, ostream& out, bool useColor) {
    auto h = board.size();
    auto w = board[0].size();
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            if (useColor) {
                if (i == 0 || j == 0) {
                    out << board[i][j] << " ";
                } else if (i == h-1 || j == w-1) {
                    out << "\033[32m" << board[i][j] << "\033[0m" << " ";
                } else { 
                    if (board[i][j] == "1") {
                        out << "\033[34m" << board[i][j] << "\033[0m" << " ";
                    } else if (board[i][j] == "0") {
                        out << "\033[31m" << board[i][j] << "\033[0m" << " ";
                    }
                }
                continue;
            }
            out << board[i][j] << " ";
        }
        out << endl;
    }
}

struct NodeBoard {
    vector<int> board;
    vector<vector<int>> path;
    bool isBound = true;
    vector<NodeBoard> children;

    NodeBoard(vector<int> boardVal, 
        vector<vector<int>> pathVal, 
        bool isBoundVal = true) 
        : board(boardVal), path(pathVal), isBound(isBoundVal) {}
};

NodeBoard CreateNodeBoard(vector<int> board, 
    vector<vector<int>> path = {}, 
    bool isBound = true) {
    
    auto newPath = path;
    newPath.push_back(board);
    return NodeBoard(board, newPath, isBound);
}

void AddChild(NodeBoard& parent, NodeBoard& child) {
    parent.children.push_back(child);
}

// Flatten
vector<int> Flatten(vector<vector<int>>& board, int h, int w) {
    vector<int> boardFlat;
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            boardFlat.push_back(board[i][j]);
        }
    }
    return boardFlat;
}

// Reshape ke vector 2D
vector<vector<int>> ReshapeTo2D(vector<int>& flattenBoard, int h, int w) {
    vector<vector<int>> boardTwoD(h, vector<int>(w));
    for (int i = 0; i < w*h; ++i) {
        boardTwoD[i / w][i % w] = flattenBoard[i];
    }
    return boardTwoD;
}

class KakurasuSolver {
    int h, w;
    vector<int> rowSum, colSum;
    unordered_set<string> visited;

    public:
        KakurasuSolver(int& height, int& width, vector<int> rowsum, vector<int> colsum)
            : h(height), w(width), rowSum(rowsum), colSum(colsum) {
            }

        vector<vector<int>> solutionPath; 
        int nodeVisited = 1;

        // Hitung 1 baris
        int CountRow(const vector<vector<int>>& board, int i) {
            int jumlah = 0;
            for (int j = 0; j < w; ++j) {
                if (board[i][j] == 1) {
                    jumlah = jumlah + (j + 1);
                }
            }
            return jumlah;
        }

        // Hitung 1 kolom
        int CountCol(const vector<vector<int>>& board, int j) {
            int jumlah = 0;
            for (int i = 0; i < h; ++i) {
                if (board[i][j] == 1) {
                    jumlah = jumlah + (i + 1);
                }
            }
            return jumlah;
        }

        // Fungsi pembatas: cek, apakah state tidak melanggar constraint
        // constraint: jumlah baris/kolom > rowSum/colSum; 
        void B(NodeBoard& node) {
            auto board = node.board;
            auto board2d = ReshapeTo2D(board, h, w);

            // jumlah 1 baris != rowSum
            for (int i = 0; i < h; ++i) {
                if (CountRow(board2d, i) > rowSum[i]) {
                    node.isBound = false;
                    return;
                }
            }

            // jumlah 1 kolom != colSum
            for (int j = 0; j < w; ++j) {
                if (CountCol(board2d, j) > colSum[j]) {
                    node.isBound = false;
                    return;
                }
            }
        } 

        // Cek apakah sudah sama semua jumlahnya dengan kunci
        bool IsSolution(vector<int>& boardFlat) {
            auto board = ReshapeTo2D(boardFlat, h, w);

            // cek per baris
            for (int i = 0; i < h; ++i) {
                if (CountRow(board, i) != rowSum[i]) {
                    return false;
                }
            }

            // cek per kolom
            for (int j = 0; j < w; ++j) {
                if (CountCol(board, j) != colSum[j]) {
                    return false;
                }
            }

            return true;
        }

        // Solusi direpresentasikan (x1, x2, x3, ..., xn) dengan n adalah banyak cell dalam board
        // dan x merupakan elemen S, S = {0, 1}
        // Kakurasu solver (DFS based)
        bool Solve(NodeBoard& node, int idx) {
            if (idx == w*h) {
                if (IsSolution(node.board)) {
                    cout << "Solusi ditemukan." << endl;
                    solutionPath = node.path;
                    return true;
                }
                return false;
            }

            for (int val: {1, 0}) {
                auto newBoard = node.board;
                newBoard[idx] = val;
                NodeBoard child = CreateNodeBoard(newBoard, node.path);

                B(child);
                AddChild(node, child);

                ++nodeVisited;
                if (nodeVisited % 25000 == 0) {
                    cout << "Node ke-" << nodeVisited << endl;
                }
                if (child.isBound) {
                    if (Solve(child, idx + 1)) {
                        return true;
                    }  
                }
            }
            return false;
        }
};

vector<vector<string>> StateChar(vector<vector<int>>& board, vector<int> rowsum, vector<int> colsum) {
    vector<string> firstRow, lastRow;
    firstRow.push_back(" ");
    lastRow.push_back(" ");
    for (int j = 0; j < board[0].size(); ++j) {
        firstRow.push_back(to_string(j + 1));
        lastRow.push_back(to_string(colsum[j]));
    }
    firstRow.push_back(" ");
    lastRow.push_back(" ");

    vector<vector<string>> solutionChar;
    solutionChar.push_back(firstRow);
    for (int i = 0; i < board.size(); ++i) {
        vector<string> dummy;
        dummy.push_back(to_string(i + 1));
        for (int j = 0; j < board[0].size(); ++j) {
            dummy.push_back(to_string(board[i][j]));
        }
        dummy.push_back(to_string(rowsum[i]));
        solutionChar.push_back(dummy);
    }
    solutionChar.push_back(lastRow);
    return solutionChar;
}

int main(int argc, char* argval[]) {
    if (argc < 2) {
        cout << "Masukkan perintah: " << argval[0] << " <nama_file>\n";
        return 1;
    }

    string fname = argval[1]; // nama file
    ifstream file(fname); // baca file

    if (!file.is_open()) {
        cerr << "Eror saat membuka file: " << fname << endl;
        return 1;
    }

    int h, w;
    vector<vector<int>> board;
    vector<int> rowSum, colSum;

    string line;
    int i = 0;
    // Masukkan konfigurasi ke dalam variabel.
    while (getline(file, line)) {
        vector<string> readLine = split(line, ' '); 
        if (i == 0) {
            h = stoi(readLine[0]); // height
            w = stoi(readLine[1]); // width

            board.resize(h);
            for (int j = 0; j < h; ++j) {
                board[j].assign(w, 0); // isi board dengan 0
            }
        }
        else if (i == 2) {
            for (int j = 0; j < h; ++j) {
                if (h != readLine.size()) {
                    cerr << "Panjang baris tidak sama dengan banyak jumlah solusi yang diberikan." << endl;
                    return 1;
                }
                rowSum.push_back(stoi(readLine[j]));
            }
        }
        else if (i == 3) {
            for (int j = 0; j < w; ++j) {
                if (w != readLine.size()) {
                    cerr << "Panjang kolom tidak sama dengan banyak jumlah solusi yang diberikan." << endl;
                    return 1;
                }
                colSum.push_back(stoi(readLine[j]));
            }
        }
        ++i;
    }

    auto boardFlatten = Flatten(board, h, w);
    auto root = CreateNodeBoard(boardFlatten);
    KakurasuSolver solver(h, w, rowSum, colSum);

    cout << "Mencari solusi..." << endl;
    auto start = high_resolution_clock::now();
    auto solution = solver.Solve(root, 0); 
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    cout << endl;

    ofstream out("../test/result_" + fname);

    for (auto& path: solver.solutionPath) {
        auto solutionPath = ReshapeTo2D(path, h, w);
        auto statePath = StateChar(solutionPath, rowSum, colSum);
        PrintBoardChar(statePath, cout, true);
        PrintBoardChar(statePath, out, false);
        cout << endl;
        out << endl;
    }

    cout << "Node visited: " << solver.nodeVisited << endl;
    cout << "Waktu eksekusi: " << duration.count() << " ms" << endl;

    out.close();
    return 0;
}
