#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <algorithm>

#include <utility>
#include <vector>
#include <string>
#include <functional>
#include <queue>

using namespace std;

// ������ Ʈ�� ��� ����ü
typedef struct HuffmanTreeNode {
	string encoded;	//���ڵ� �� ��
	int freq;		//�󵵼�
	unsigned char ascii_code;	//ASCII CODE
	HuffmanTreeNode* leftnode;	//���� ��� 
	HuffmanTreeNode* rightnode;	//������ ���
} HuffmanTreeNode;


// ������ Ʈ�� ����
HuffmanTreeNode* HuffmanTree_Insert(HuffmanTreeNode* u, HuffmanTreeNode* v) {
	HuffmanTreeNode *pt;
	pt = new HuffmanTreeNode;	//�θ� ��� ����
	pt->ascii_code = NULL;
	pt->freq = u->freq + v->freq;	//�󵵼��� ���Ѵ�.
	pt->leftnode = u;			//��, �� ��� ����
	pt->rightnode = v;
	return pt;		//�θ��� ��ȯ
}


// ������ ���̺� ����
int MakeTable(HuffmanTreeNode* t, int &tnum) {
	FILE * huff_fp = fopen("huffman_table.hbs", "r+b");	//�̾��
	if (huff_fp == NULL)	huff_fp = fopen("huffman_table.hbs", "a+b");	//�������� �ʴ´ٸ� ����
	unsigned char bit_length = t->encoded.length();	//string�� ���� == encoded bit length
	unsigned char * temp_c;
	temp_c = new unsigned char[bit_length + 1];
	strcpy((char *)temp_c, t->encoded.c_str());		//string to unsigned char arr
	if (tnum != 0) {
		unsigned char ascii_hi = t->ascii_code >> tnum;
		unsigned char ascii_lo = t->ascii_code << (8 - tnum);	//ascii �ڸ�
		unsigned char bit_length_hi = bit_length >> tnum;
		unsigned char bit_length_lo = bit_length << (8 - tnum);	//bitlength �ڸ�
		fseek(huff_fp, 0, SEEK_END);
		fseek(huff_fp, -1L, SEEK_CUR);
		unsigned char last_char = fgetc(huff_fp);
		unsigned char temp = NULL;
		fseek(huff_fp, -1L, SEEK_CUR);
		temp += last_char;
		temp += ascii_hi;
		fprintf(huff_fp, "%c", temp);
		fprintf(huff_fp, "%c", ascii_lo + bit_length_hi);
		bool end_flag = false;
		int codeword_count = 0;
		for (int i = 0; i < 8 - tnum; i++) {	//codeword �պκ�
			if (temp_c[codeword_count] == NULL) {
				end_flag = true;
				break;
			}
			if (temp_c[codeword_count] & 0x01 == 0x01) {
				bit_length_lo += 0x01 << (7 - tnum - i);
			}
			else {
				bit_length_lo += 0x00 << (7 - tnum - i);
			}
			codeword_count++;
		}
		fprintf(huff_fp, "%c", bit_length_lo);
		int temp_cc = codeword_count;
		if (end_flag == false) {
			unsigned char codeword_lo = NULL;
			for (int j = 0; j < (bit_length - temp_cc - 1) / 8 + 1; j++) {	//codeword�޺κ� + stuffing bit
				codeword_lo = NULL;
				if (temp_c[codeword_count] == NULL) break;
				for (int i = 0; i < 8; i++) {
					if (temp_c[codeword_count] == NULL) break;
					if (temp_c[codeword_count] & 0x01 == 0x01) {
						codeword_lo += 0x01 << (7 - i);
					}
					else {
						codeword_lo += 0x00 << (7 - i);
					}
					codeword_count++;
				}
				fprintf(huff_fp, "%c", codeword_lo);
			}
		}
	}
	else {
		fseek(huff_fp, 0, SEEK_END);
		fprintf(huff_fp, "%c", t->ascii_code);
		fprintf(huff_fp, "%c", bit_length);
		unsigned char codeword = NULL;
		int codeword_count = 0;
		for (int j = 0; j < (bit_length - 1) / 8 + 1; j++) {
			codeword = NULL;
			for (int i = 0; i < 8; i++) {
				if (temp_c[codeword_count] == NULL) break;	//temp_c�� ���� 8bit
				if (temp_c[codeword_count] & 0x01 == 0x01) {
					codeword += 0x01 << (7 - i);		//�̸� concatenate
				}
				else {
					codeword += 0x00 << (7 - i);
				}
				codeword_count++;	//�迭 ++
			}
			fprintf(huff_fp, "%c", codeword);	//8������ ���
		}
	}
	fclose(huff_fp);
	tnum += bit_length % 8;
	while (1) {
		if (tnum >= 8) tnum -= 8;
		else break;
	}
	return tnum;
}


// ������ Ʈ�� ������ȸ
void Inorder_traversal(HuffmanTreeNode* t, string num, int &tnum) {
	if (t != NULL) {
		Inorder_traversal(t->leftnode, num + "0", tnum);
		if (t->ascii_code != NULL) {
			t->encoded = num;
			cout << "'" << t->ascii_code << "'\t'" << t->freq << "'\t'"
				<< t->encoded << "'" << endl;
			tnum = MakeTable(t, tnum);
		}
		Inorder_traversal(t->rightnode, num + "1", tnum);
	}
}


// ���ڵ�
void Encoding(unsigned char * codeword, int arr_size, int &tnum, unsigned char length) {
	FILE * code_fp = fopen("huffman_code.hbs", "r+b");	//���
	if (code_fp == NULL) code_fp = fopen("huffman_code.hbs", "a+b");
	if (tnum == 0) {
		fseek(code_fp, 0, SEEK_END);
		for (int i = 0; i < arr_size; i++) {
			fprintf(code_fp, "%c", codeword[i]);
		}
	}
	else {
		unsigned char temp_ov = NULL;
		bool ov_flag = false;
		if (arr_size == 1) {	//bit �� ª�� ���
			if (length + tnum > 8) {	//bit loss�� �߻��ϴ� ���
				temp_ov = codeword[0];
				temp_ov = temp_ov >> 8 - length;
				temp_ov = temp_ov << 16 - length - tnum;	//loss bit temporary
				ov_flag = true;
			}
		}
		else {	//bit�� �� ���
			for (int i = arr_size - 1; i > 0; i--) {
				if (i == arr_size - 1) {	//���� ���� ��Ʈ�� ���
					if (length + tnum > 8) {	//shift bit loss�� �߻��� ���
						temp_ov = codeword[i];
						temp_ov = temp_ov >> 8 - length;
						temp_ov = temp_ov << 16 - length - tnum;	//loss bit temporary
						ov_flag = true;
					}
				}
				codeword[i] = codeword[i] >> tnum;
				codeword[i] += codeword[i - 1] << 8 - tnum;
			}
		}
		codeword[0] = codeword[0] >> tnum;		//codeword������

		fseek(code_fp, 0, SEEK_END);
		fseek(code_fp, -1L, SEEK_CUR);
		unsigned char result = fgetc(code_fp);
		fseek(code_fp, -1L, SEEK_CUR);
		result += codeword[0];
		fprintf(code_fp, "%c", result);
		for (int i = 1; i < arr_size; i++) {	//������ ���
			fprintf(code_fp, "%c", codeword[i]);
		}
		if (ov_flag == true) {
			fprintf(code_fp, "%c", temp_ov);
		}
	}
	tnum += length;
	if (tnum >= 8)	tnum -= 8;
	fclose(code_fp);
}


// ������ ���̺��� �ش� �� ã��
void GetFromTable(unsigned char ch, int &wr_tnum) {
	FILE * table_fp = fopen("huffman_table.hbs", "rb");	//������ ���̺�

	int tnum = 0;	//���� ��ġ
	unsigned char ascii_table, ascii_table_hi, ascii_table_lo = NULL;	//ascii
	unsigned char bl_table, bl_table_hi, bl_table_lo = NULL;	//bit length
	unsigned char * codeword;	//encoded code

	while (1) {
		ascii_table_hi = fgetc(table_fp) << tnum;		// N = table_fp ��ġ
		ascii_table_lo = fgetc(table_fp) >> (8 - tnum);	// N+1
		if (feof(table_fp)) {	//if N+1 == null -> EOF
			break;
		}
		ascii_table = ascii_table_hi + ascii_table_lo;	//�о�� table���� ascii �� ����
		fseek(table_fp, -1L, SEEK_CUR);		// N

		bl_table_hi = fgetc(table_fp) << tnum; // N+1
		bl_table_lo = fgetc(table_fp) >> (8 - tnum);	// N+2
		bl_table = bl_table_hi + bl_table_lo;	//bit length�� ����
		fseek(table_fp, -1L, SEEK_CUR);		// N+1

		int arr_size = (bl_table - 1) / 8 + 1;
		codeword = new unsigned char[arr_size];
		memset(codeword, 0, arr_size * sizeof(unsigned char));
		unsigned char last_bit_length = NULL;
		if (ascii_table == ch) {	//ã�� ���
			int code_cnt = bl_table;
			for (int i = 0; i < arr_size; i++) {	//get codeword
				unsigned char temp_cw = NULL;
				if (tnum + code_cnt > 8) {		//bit �� �����������
					temp_cw = fgetc(table_fp) << tnum;	//�պκ�
					codeword[i] += temp_cw;
					temp_cw = fgetc(table_fp) >> (8 - tnum);	//�޺κ�
					code_cnt -= 8 - tnum;
					if (tnum - code_cnt < 0) {	//�ڿ� �� �������
						codeword[i] += temp_cw;
						fseek(table_fp, -1L, SEEK_CUR);
						code_cnt -= tnum;
					}
					else {	//�ڿ� �ȳ������
						last_bit_length = (8 - tnum) + code_cnt;
						unsigned char shamt = 8 - last_bit_length;
						temp_cw = (temp_cw & (0xff << shamt));	//�� �������� ä��
						codeword[i] += temp_cw;
					}
				}
				else {	//�Ȼ����������
					temp_cw = fgetc(table_fp) << tnum;
					temp_cw = (temp_cw & (0xff << (8 - code_cnt)));	//�� �������� ä��
					codeword[i] += temp_cw;
					last_bit_length = code_cnt;
				}
			}
			Encoding(codeword, arr_size, wr_tnum, last_bit_length);	//���� ���
			break;
		}
		else {	//�ƴѰ��
			tnum += bl_table;
			while (1) {
				if (tnum >= 8) {
					fseek(table_fp, 1L, SEEK_CUR);		// N+1
					tnum -= 8;
				}
				else break;
			}
			continue;
		}
	}
	fclose(table_fp);
}


int main() {
	// Step 1. Load File
	FILE * input = fopen("input.txt", "r");
	unsigned char chTemp;
	int ascii[128] = { 0, };	//count ASCII
	if (!input)
		printf("���� �ȿ��� \n");
	// �ƽ�Ű ī��Ʈ
	while (1)
	{
		chTemp = fgetc(input);
		if (feof(input)) {
			break;
		}
		ascii[chTemp]++;
	}
	// Step 1.1 End Of Data ����
	unsigned char end_of_data = 0x7f;
	ascii[end_of_data]++;

	// Step 2. Min-Heap�� Insert. Used Priority_queue	
	priority_queue< pair<int, HuffmanTreeNode*>, vector< pair<int, HuffmanTreeNode*> >, greater< pair<int, HuffmanTreeNode*> > > pq;
	for (int i = 0; i < 128; i++) {
		if (ascii[i] != 0) {
			HuffmanTreeNode *t;
			t = new HuffmanTreeNode;	//��� ����
			t->ascii_code = i;
			t->freq = ascii[i];
			t->leftnode = NULL;
			t->rightnode = NULL;
			pq.push(make_pair(ascii[i], t));
		}
	}

	// Step 3. Make Huffman Tree
	HuffmanTreeNode* root;
	while (1) {
		pair<int, HuffmanTreeNode*> temp;
		if (pq.size() == 1) {
			temp = pq.top();
			root = temp.second;
			break;
		}
		temp = pq.top();
		pq.pop();
		HuffmanTreeNode *u, *v, *p;
		u = new HuffmanTreeNode;	//u��� ����
		u->ascii_code = temp.second->ascii_code;
		u->freq = temp.first;
		u->leftnode = temp.second->leftnode;
		u->rightnode = temp.second->rightnode;

		temp = pq.top();
		pq.pop();
		v = new HuffmanTreeNode;	//v��� ����
		v->ascii_code = temp.second->ascii_code;
		v->freq = temp.first;
		v->leftnode = temp.second->leftnode;
		v->rightnode = temp.second->rightnode;


		p = HuffmanTree_Insert(u, v);
		pq.push(make_pair(p->freq, p));
	}

	// Step 4. Make Huffman Table
	string encoded_num;
	int tnum = 0;
	Inorder_traversal(root, encoded_num, tnum);
	fclose(input);

	// Step 5. Encoding	
	input = fopen("input.txt", "r");
	if (!input)
		printf("���� �ȿ��� \n");
	tnum = 0;
	while (1)
	{
		chTemp = fgetc(input);
		if (feof(input)) {
			GetFromTable(end_of_data, tnum);
			break;
		}
		GetFromTable(chTemp, tnum);
	}

	fclose(input);
	return 0;
}