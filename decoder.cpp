#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <string>

using namespace std;


// 허프만 트리 노드 구조체
typedef struct HuffmanTreeNode {
	string encoded;	//인코딩 된 수
	unsigned char ascii_code;	//ASCII CODE
} HuffmanTreeNode;

// unsigned char 를 string으로 bit단위 읽어가며 바꾸는 함수
void UCharToString(string &result, unsigned char ch, unsigned char length) {
	for (int j = 7; j > 7 - length; j--) {
		if ((0x01 & (ch >> j)) == 0x01) {
			result += '1';
		}
		else {
			result += '0';
		}
	}
}

// HuffmanTreeNode에 값을 넣어주는 함수
void InsertNode(vector<HuffmanTreeNode*> &v, int size, 
	unsigned char * cw, unsigned char lbl, unsigned char ascii){
	string result;
	HuffmanTreeNode *t;
	t = new HuffmanTreeNode;
	t->ascii_code = ascii;
	for (int i = 0; i < size; i++) {	//8bit씩을 1size로 보고 8bit이상인경우 반복문 돌림
		if (i == size - 1) {
			UCharToString(result, cw[i], lbl);
		}
		else {
			UCharToString(result, cw[i], 8);
		}
	}
	t->encoded = result;
	v.push_back(t);
}

// Huffman_table.hbs를 읽어 Table(vector)를 재생성 하는 함수
void MakeTableFromEncoded(vector<HuffmanTreeNode*> &v) {
	FILE * table_fp = fopen("huffman_table.hbs", "rb");
	if (!table_fp) {
		printf("파일 존재하지 않음\n");
	}
	int tnum = 0;	//끊는 위치
	unsigned char ascii_table, ascii_table_hi, ascii_table_lo = NULL;	//ascii
	unsigned char bl_table, bl_table_hi, bl_table_lo = NULL;	//bit length
	unsigned char * codeword;	//encoded code
	unsigned char last_bit_length = NULL;

	while (1) {
		ascii_table_hi = fgetc(table_fp) << tnum;		// N = table_fp 위치
		ascii_table_lo = fgetc(table_fp) >> (8 - tnum);	// N+1
		if (feof(table_fp)) {	//if N+1 == null -> EOF
			break;
		}
		ascii_table = ascii_table_hi + ascii_table_lo;
		fseek(table_fp, -1L, SEEK_CUR);		// N

		bl_table_hi = fgetc(table_fp) << tnum; // N+1
		bl_table_lo = fgetc(table_fp) >> (8 - tnum);	// N+2
		bl_table = bl_table_hi + bl_table_lo;	//bit length값 결정
		fseek(table_fp, -1L, SEEK_CUR);		// N+1

		int arr_size = (bl_table - 1) / 8 + 1;
		codeword = new unsigned char[arr_size];
		memset(codeword, 0, arr_size * sizeof(unsigned char));
		int code_cnt = bl_table;
		for (int i = 0; i < arr_size; i++) {
			unsigned char temp_cw = NULL;
			if (tnum + code_cnt > 8) {		//bit 가 삐져나간경우
				temp_cw = fgetc(table_fp) << tnum;	//앞부분
				codeword[i] += temp_cw;
				temp_cw = fgetc(table_fp) >> (8 - tnum);	//뒷부분
				code_cnt -= 8 - tnum;
				if (tnum - code_cnt < 0) {	//뒤에 더 남은경우
					code_cnt -= tnum;
					codeword[i] += temp_cw;
					fseek(table_fp, -1L, SEEK_CUR);
				}
				else {	//뒤에 안남은경우
					last_bit_length = (8 - tnum) + code_cnt;
					unsigned char shamt = 8 - last_bit_length;
					temp_cw = (temp_cw & (0xff << shamt));	//다 왼쪽으로 채움
					codeword[i] += temp_cw;
				}
			}
			else {	//안삐져 나오는 경우
				temp_cw = fgetc(table_fp) << tnum;
				temp_cw = (temp_cw & (0xff << (8 - code_cnt)));	//다 왼쪽으로 채움
				codeword[i] += temp_cw;
				last_bit_length = code_cnt;
			}
		}
		// Huffman struct 생성
		InsertNode(v, arr_size, codeword, last_bit_length, ascii_table);

		tnum += bl_table;
		if (tnum % 8 != 0) {
			fseek(table_fp, -1L, SEEK_CUR);		// N -= 1
		}
		while (1) {
			if (tnum >= 8)
				tnum -= 8;
			else break;
		}
	}
	fclose(table_fp);
}

// vector에서 원하는 string bitstream찾는 과정
unsigned char SearchVector(vector<HuffmanTreeNode*> &v, string s) {
	int cnt = 0;
	while (1) {
		if (v[cnt]->encoded == s) {
			return v[cnt]->ascii_code;	//찾으면 반환
		}
		else {
			if (v.back() == v[cnt]) {	//못찾는 경우(있을 수 없음)
				break;
			}
			cnt++;
		}
	}
	return NULL;	//따라서 오류반환
}

// output.txt에 결과 출력
void WriteASCII(unsigned char ch) {
	FILE * decoded_fp = fopen("output.txt", "a+");
	if (!decoded_fp) {
		printf("파일 오류\n");
	}
	fprintf(decoded_fp, "%c", ch);	//a+모드로 이어쓰기만 해주면 된다.
	fclose(decoded_fp);
}

// Decode main
void Decode(vector<HuffmanTreeNode*> &v, unsigned char eod) {
	FILE * code_fp = fopen("huffman_code.hbs", "rb");
	if (!code_fp) {
		printf("파일이 존재하지 않음\n");
	}

	unsigned char code, code_hi, code_lo = NULL;	// 8bit 읽음
	unsigned char temp = NULL;	//잘라서 볼 temporary
	unsigned char decoded = NULL;
	int tnum = 0;
	bool find_flag = false;
	string s, temp_s;
	while (1) {
		code_hi = fgetc(code_fp) << tnum;
		code_lo = fgetc(code_fp) >> (8 - tnum);
		fseek(code_fp, -1L, SEEK_CUR);
		code = code_hi + code_lo;
 		for (int i = 7; i >= 0; i--) {	//8번 돌려서 Search값, 원하는값 나올때까지 반복
			temp_s = s+"";
			UCharToString(temp_s, code, 8 - i);
			decoded = SearchVector(v, temp_s);	//찾는 값이 없음
			if (decoded == eod) {	//EOD읽음
				return;
			}
			if (decoded != NULL) {
				s = "";
				temp_s = "";
				find_flag = true;
				WriteASCII(decoded);	//decoded된 ASCII값 출력
				tnum += (8 - i);
				fseek(code_fp, -1L, SEEK_CUR);		// N -= 1
				while (1) {
					if (tnum >= 8) {
						fseek(code_fp, 1L, SEEK_CUR);	//파일 포인터 재조정
						tnum -= 8;
					}
					else break;
				}
				break;
			}
		}
		if (find_flag == false) {	//8bit넘어가는 경우 s에 기존 8bit저장
			s += temp_s;
		}
		find_flag = false;	//flag초기화
	}
	fclose(code_fp);
}


int main() {
	vector<HuffmanTreeNode*> table;
	// Step 1. huffman_table.hbs를 갖고 table을 만든다.
	MakeTableFromEncoded(table);

	int cnt = 0;
	while (1) {
		cout << "'" << table[cnt]->ascii_code << "'\t'"	//vector 확인
			<< table[cnt]->encoded << "'" << endl;		//inorder로 저장되어있음
		if (table.back() == table[cnt]) {
			break;
		}
		cnt++;
	}

	// Step 2. vector huffman table을 통하여 decode
	unsigned char end_of_data = 0x7f;
	Decode(table, end_of_data);
}

