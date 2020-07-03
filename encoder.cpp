#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <algorithm>

#include <utility>
#include <vector>
#include <string>
#include <functional>
#include <queue>

using namespace std;

// 허프만 트리 노드 구조체
typedef struct HuffmanTreeNode {
	string encoded;	//인코딩 된 수
	int freq;		//빈도수
	unsigned char ascii_code;	//ASCII CODE
	HuffmanTreeNode* leftnode;	//왼쪽 노드 
	HuffmanTreeNode* rightnode;	//오른쪽 노드
} HuffmanTreeNode;


// 허프만 트리 삽입
HuffmanTreeNode* HuffmanTree_Insert(HuffmanTreeNode* u, HuffmanTreeNode* v) {
	HuffmanTreeNode *pt;
	pt = new HuffmanTreeNode;	//부모 노드 설정
	pt->ascii_code = NULL;
	pt->freq = u->freq + v->freq;	//빈도수를 합한다.
	pt->leftnode = u;			//좌, 우 노드 설정
	pt->rightnode = v;
	return pt;		//부모노드 반환
}


// 허프만 테이블 생성
int MakeTable(HuffmanTreeNode* t, int &tnum) {
	FILE * huff_fp = fopen("huffman_table.hbs", "r+b");	//이어쓰기
	if (huff_fp == NULL)	huff_fp = fopen("huffman_table.hbs", "a+b");	//존재하지 않는다면 생성
	unsigned char bit_length = t->encoded.length();	//string의 길이 == encoded bit length
	unsigned char * temp_c;
	temp_c = new unsigned char[bit_length + 1];
	strcpy((char *)temp_c, t->encoded.c_str());		//string to unsigned char arr
	if (tnum != 0) {
		unsigned char ascii_hi = t->ascii_code >> tnum;
		unsigned char ascii_lo = t->ascii_code << (8 - tnum);	//ascii 자름
		unsigned char bit_length_hi = bit_length >> tnum;
		unsigned char bit_length_lo = bit_length << (8 - tnum);	//bitlength 자름
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
		for (int i = 0; i < 8 - tnum; i++) {	//codeword 앞부분
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
			for (int j = 0; j < (bit_length - temp_cc - 1) / 8 + 1; j++) {	//codeword뒷부분 + stuffing bit
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
				if (temp_c[codeword_count] == NULL) break;	//temp_c는 개당 8bit
				if (temp_c[codeword_count] & 0x01 == 0x01) {
					codeword += 0x01 << (7 - i);		//이를 concatenate
				}
				else {
					codeword += 0x00 << (7 - i);
				}
				codeword_count++;	//배열 ++
			}
			fprintf(huff_fp, "%c", codeword);	//8개마다 출력
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


// 허프만 트리 중위순회
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


// 인코딩
void Encoding(unsigned char * codeword, int arr_size, int &tnum, unsigned char length) {
	FILE * code_fp = fopen("huffman_code.hbs", "r+b");	//결과
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
		if (arr_size == 1) {	//bit 가 짧은 경우
			if (length + tnum > 8) {	//bit loss가 발생하는 경우
				temp_ov = codeword[0];
				temp_ov = temp_ov >> 8 - length;
				temp_ov = temp_ov << 16 - length - tnum;	//loss bit temporary
				ov_flag = true;
			}
		}
		else {	//bit가 긴 경우
			for (int i = arr_size - 1; i > 0; i--) {
				if (i == arr_size - 1) {	//가장 하위 비트의 경우
					if (length + tnum > 8) {	//shift bit loss가 발생할 경우
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
		codeword[0] = codeword[0] >> tnum;		//codeword재정비

		fseek(code_fp, 0, SEEK_END);
		fseek(code_fp, -1L, SEEK_CUR);
		unsigned char result = fgetc(code_fp);
		fseek(code_fp, -1L, SEEK_CUR);
		result += codeword[0];
		fprintf(code_fp, "%c", result);
		for (int i = 1; i < arr_size; i++) {	//나머지 출력
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


// 허프만 테이블에서 해당 값 찾기
void GetFromTable(unsigned char ch, int &wr_tnum) {
	FILE * table_fp = fopen("huffman_table.hbs", "rb");	//허프만 테이블

	int tnum = 0;	//끊는 위치
	unsigned char ascii_table, ascii_table_hi, ascii_table_lo = NULL;	//ascii
	unsigned char bl_table, bl_table_hi, bl_table_lo = NULL;	//bit length
	unsigned char * codeword;	//encoded code

	while (1) {
		ascii_table_hi = fgetc(table_fp) << tnum;		// N = table_fp 위치
		ascii_table_lo = fgetc(table_fp) >> (8 - tnum);	// N+1
		if (feof(table_fp)) {	//if N+1 == null -> EOF
			break;
		}
		ascii_table = ascii_table_hi + ascii_table_lo;	//읽어온 table에서 ascii 값 결정
		fseek(table_fp, -1L, SEEK_CUR);		// N

		bl_table_hi = fgetc(table_fp) << tnum; // N+1
		bl_table_lo = fgetc(table_fp) >> (8 - tnum);	// N+2
		bl_table = bl_table_hi + bl_table_lo;	//bit length값 결정
		fseek(table_fp, -1L, SEEK_CUR);		// N+1

		int arr_size = (bl_table - 1) / 8 + 1;
		codeword = new unsigned char[arr_size];
		memset(codeword, 0, arr_size * sizeof(unsigned char));
		unsigned char last_bit_length = NULL;
		if (ascii_table == ch) {	//찾던 결과
			int code_cnt = bl_table;
			for (int i = 0; i < arr_size; i++) {	//get codeword
				unsigned char temp_cw = NULL;
				if (tnum + code_cnt > 8) {		//bit 가 삐져나간경우
					temp_cw = fgetc(table_fp) << tnum;	//앞부분
					codeword[i] += temp_cw;
					temp_cw = fgetc(table_fp) >> (8 - tnum);	//뒷부분
					code_cnt -= 8 - tnum;
					if (tnum - code_cnt < 0) {	//뒤에 더 남은경우
						codeword[i] += temp_cw;
						fseek(table_fp, -1L, SEEK_CUR);
						code_cnt -= tnum;
					}
					else {	//뒤에 안남은경우
						last_bit_length = (8 - tnum) + code_cnt;
						unsigned char shamt = 8 - last_bit_length;
						temp_cw = (temp_cw & (0xff << shamt));	//다 왼쪽으로 채움
						codeword[i] += temp_cw;
					}
				}
				else {	//안삐져나간경우
					temp_cw = fgetc(table_fp) << tnum;
					temp_cw = (temp_cw & (0xff << (8 - code_cnt)));	//다 왼쪽으로 채움
					codeword[i] += temp_cw;
					last_bit_length = code_cnt;
				}
			}
			Encoding(codeword, arr_size, wr_tnum, last_bit_length);	//파일 출력
			break;
		}
		else {	//아닌결과
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
		printf("파일 안열림 \n");
	// 아스키 카운트
	while (1)
	{
		chTemp = fgetc(input);
		if (feof(input)) {
			break;
		}
		ascii[chTemp]++;
	}
	// Step 1.1 End Of Data 설정
	unsigned char end_of_data = 0x7f;
	ascii[end_of_data]++;

	// Step 2. Min-Heap에 Insert. Used Priority_queue	
	priority_queue< pair<int, HuffmanTreeNode*>, vector< pair<int, HuffmanTreeNode*> >, greater< pair<int, HuffmanTreeNode*> > > pq;
	for (int i = 0; i < 128; i++) {
		if (ascii[i] != 0) {
			HuffmanTreeNode *t;
			t = new HuffmanTreeNode;	//노드 설정
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
		u = new HuffmanTreeNode;	//u노드 설정
		u->ascii_code = temp.second->ascii_code;
		u->freq = temp.first;
		u->leftnode = temp.second->leftnode;
		u->rightnode = temp.second->rightnode;

		temp = pq.top();
		pq.pop();
		v = new HuffmanTreeNode;	//v노드 설정
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
		printf("파일 안열림 \n");
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