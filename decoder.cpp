#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <string>

using namespace std;


// ������ Ʈ�� ��� ����ü
typedef struct HuffmanTreeNode {
	string encoded;	//���ڵ� �� ��
	unsigned char ascii_code;	//ASCII CODE
} HuffmanTreeNode;

// unsigned char �� string���� bit���� �о�� �ٲٴ� �Լ�
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

// HuffmanTreeNode�� ���� �־��ִ� �Լ�
void InsertNode(vector<HuffmanTreeNode*> &v, int size, 
	unsigned char * cw, unsigned char lbl, unsigned char ascii){
	string result;
	HuffmanTreeNode *t;
	t = new HuffmanTreeNode;
	t->ascii_code = ascii;
	for (int i = 0; i < size; i++) {	//8bit���� 1size�� ���� 8bit�̻��ΰ�� �ݺ��� ����
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

// Huffman_table.hbs�� �о� Table(vector)�� ����� �ϴ� �Լ�
void MakeTableFromEncoded(vector<HuffmanTreeNode*> &v) {
	FILE * table_fp = fopen("huffman_table.hbs", "rb");
	if (!table_fp) {
		printf("���� �������� ����\n");
	}
	int tnum = 0;	//���� ��ġ
	unsigned char ascii_table, ascii_table_hi, ascii_table_lo = NULL;	//ascii
	unsigned char bl_table, bl_table_hi, bl_table_lo = NULL;	//bit length
	unsigned char * codeword;	//encoded code
	unsigned char last_bit_length = NULL;

	while (1) {
		ascii_table_hi = fgetc(table_fp) << tnum;		// N = table_fp ��ġ
		ascii_table_lo = fgetc(table_fp) >> (8 - tnum);	// N+1
		if (feof(table_fp)) {	//if N+1 == null -> EOF
			break;
		}
		ascii_table = ascii_table_hi + ascii_table_lo;
		fseek(table_fp, -1L, SEEK_CUR);		// N

		bl_table_hi = fgetc(table_fp) << tnum; // N+1
		bl_table_lo = fgetc(table_fp) >> (8 - tnum);	// N+2
		bl_table = bl_table_hi + bl_table_lo;	//bit length�� ����
		fseek(table_fp, -1L, SEEK_CUR);		// N+1

		int arr_size = (bl_table - 1) / 8 + 1;
		codeword = new unsigned char[arr_size];
		memset(codeword, 0, arr_size * sizeof(unsigned char));
		int code_cnt = bl_table;
		for (int i = 0; i < arr_size; i++) {
			unsigned char temp_cw = NULL;
			if (tnum + code_cnt > 8) {		//bit �� �����������
				temp_cw = fgetc(table_fp) << tnum;	//�պκ�
				codeword[i] += temp_cw;
				temp_cw = fgetc(table_fp) >> (8 - tnum);	//�޺κ�
				code_cnt -= 8 - tnum;
				if (tnum - code_cnt < 0) {	//�ڿ� �� �������
					code_cnt -= tnum;
					codeword[i] += temp_cw;
					fseek(table_fp, -1L, SEEK_CUR);
				}
				else {	//�ڿ� �ȳ������
					last_bit_length = (8 - tnum) + code_cnt;
					unsigned char shamt = 8 - last_bit_length;
					temp_cw = (temp_cw & (0xff << shamt));	//�� �������� ä��
					codeword[i] += temp_cw;
				}
			}
			else {	//�Ȼ��� ������ ���
				temp_cw = fgetc(table_fp) << tnum;
				temp_cw = (temp_cw & (0xff << (8 - code_cnt)));	//�� �������� ä��
				codeword[i] += temp_cw;
				last_bit_length = code_cnt;
			}
		}
		// Huffman struct ����
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

// vector���� ���ϴ� string bitstreamã�� ����
unsigned char SearchVector(vector<HuffmanTreeNode*> &v, string s) {
	int cnt = 0;
	while (1) {
		if (v[cnt]->encoded == s) {
			return v[cnt]->ascii_code;	//ã���� ��ȯ
		}
		else {
			if (v.back() == v[cnt]) {	//��ã�� ���(���� �� ����)
				break;
			}
			cnt++;
		}
	}
	return NULL;	//���� ������ȯ
}

// output.txt�� ��� ���
void WriteASCII(unsigned char ch) {
	FILE * decoded_fp = fopen("output.txt", "a+");
	if (!decoded_fp) {
		printf("���� ����\n");
	}
	fprintf(decoded_fp, "%c", ch);	//a+���� �̾�⸸ ���ָ� �ȴ�.
	fclose(decoded_fp);
}

// Decode main
void Decode(vector<HuffmanTreeNode*> &v, unsigned char eod) {
	FILE * code_fp = fopen("huffman_code.hbs", "rb");
	if (!code_fp) {
		printf("������ �������� ����\n");
	}

	unsigned char code, code_hi, code_lo = NULL;	// 8bit ����
	unsigned char temp = NULL;	//�߶� �� temporary
	unsigned char decoded = NULL;
	int tnum = 0;
	bool find_flag = false;
	string s, temp_s;
	while (1) {
		code_hi = fgetc(code_fp) << tnum;
		code_lo = fgetc(code_fp) >> (8 - tnum);
		fseek(code_fp, -1L, SEEK_CUR);
		code = code_hi + code_lo;
 		for (int i = 7; i >= 0; i--) {	//8�� ������ Search��, ���ϴ°� ���ö����� �ݺ�
			temp_s = s+"";
			UCharToString(temp_s, code, 8 - i);
			decoded = SearchVector(v, temp_s);	//ã�� ���� ����
			if (decoded == eod) {	//EOD����
				return;
			}
			if (decoded != NULL) {
				s = "";
				temp_s = "";
				find_flag = true;
				WriteASCII(decoded);	//decoded�� ASCII�� ���
				tnum += (8 - i);
				fseek(code_fp, -1L, SEEK_CUR);		// N -= 1
				while (1) {
					if (tnum >= 8) {
						fseek(code_fp, 1L, SEEK_CUR);	//���� ������ ������
						tnum -= 8;
					}
					else break;
				}
				break;
			}
		}
		if (find_flag == false) {	//8bit�Ѿ�� ��� s�� ���� 8bit����
			s += temp_s;
		}
		find_flag = false;	//flag�ʱ�ȭ
	}
	fclose(code_fp);
}


int main() {
	vector<HuffmanTreeNode*> table;
	// Step 1. huffman_table.hbs�� ���� table�� �����.
	MakeTableFromEncoded(table);

	int cnt = 0;
	while (1) {
		cout << "'" << table[cnt]->ascii_code << "'\t'"	//vector Ȯ��
			<< table[cnt]->encoded << "'" << endl;		//inorder�� ����Ǿ�����
		if (table.back() == table[cnt]) {
			break;
		}
		cnt++;
	}

	// Step 2. vector huffman table�� ���Ͽ� decode
	unsigned char end_of_data = 0x7f;
	Decode(table, end_of_data);
}

