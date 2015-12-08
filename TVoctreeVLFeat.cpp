#include "TVoctreeVLFeat.h"

#include <iostream>
#include <string>

using namespace std;

#define VL_FEAT_HIKM_TREE_MAGIC 2494285466



int read_DSC_from_flicker1M(string readFolderPath, string dsc_type, unsigned char* descs, vl_size &numTotalDesc, vl_size maxTotalDesc)
{
	vector<string> folderList;
	get_folder_list(readFolderPath.c_str(),folderList);
	int numOfParallel = 32;
	omp_set_dynamic(0);     // Explicitly disable dynamic teams
	omp_set_num_threads(32); // Use 4 threads for all consecutive parallel regions		
	for (int k = 0; k < folderList.size(); k++)
	{
		string subFolderPath = readFolderPath + "/" + folderList[k] + "/" + dsc_type;
		vector<string> subFolderList;
		get_folder_list(subFolderPath.c_str(), subFolderList);
		clock_t start = clock();
		double duration;
		
		for (int l = 0; l < subFolderList.size(); l++ )
		{
			string subsubFolderPath = subFolderPath + "/" + subFolderList[l];
			vector<string> dscList;
			get_directory_dsc(subsubFolderPath.c_str(), dscList);
			int cores = 16;
			#pragma omp parallel for
			for (int m = 0; m < dscList.size(); m++)
			{
//				unsigned char** descs_fork = new unsigned char*[16];
//				unsigned int numDesc_fork[16];
////////////////////////////////////////////////////////////////////////////
//				#pragma omp parallel for
//				for (int p = 0; p < cores; p++)
//				{
//					string dscPath = subsubFolderPath + "/" + dscList[ m + p];
//					type1_descriptors dscFile(dscPath,FeatureType::AKAZE_FEATS);
//					dscFile.read_dsc_v1();
//					numDesc_fork[p] = dscFile.get_num_descriptors();
//					descs_fork[p] = new unsigned char [numDesc_fork[p]*61];
//					memcpy(descs_fork[p], dscFile.get_data(), (numDesc_fork[p])*61*sizeof(unsigned char));
//				}
////////////////////////////////////////////////////////////////////////////
//
//				for (int p = 0; p < cores; p++)
//				{
//					memcpy(descs+(numTotalDesc*61), descs_fork[p], (numDesc_fork[p])*61*sizeof(unsigned char));
//					numTotalDesc += numDesc_fork[p];
//					delete[] descs_fork[p];
//					descs_fork[p] = NULL;
//				}
//				delete[] descs_fork;
//				descs_fork = NULL;

				string dscPath = subsubFolderPath + "/" + dscList[ m];
				uchar_descriptors dscFile(dscPath.c_str(),FeatureType::AKAZE_FEATS);
				dscFile.read_dsc_v1();
				unsigned int numDesc = dscFile.get_num_descriptors();
				memcpy(descs+(numTotalDesc*61), dscFile.get_data(), (numDesc)*61*sizeof(unsigned char));
				numTotalDesc += numDesc;
				if(	m % 1000 == 0 ) printf("\rProcess Rate in Folder: %.2f%%, Total Rate: %.2f",100.0*m/10000,( m*1.00 + (10000*l + (100000*k)))/10000);	
			}
			
		}
		printf("\nTotal num of Descriptors: %d .", static_cast<int>(numTotalDesc));	
		duration = ( std::clock() - start ) / double(CLOCKS_PER_SEC);
		printf("\nDuration: %f\n\n", duration);
	}

	return 1;

}

TVoctreeVLFeat::TVoctreeVLFeat(void){
	m_init = 0;
}

TVoctreeVLFeat::~TVoctreeVLFeat(void) {
	if (m_init == 1) {
		clean();
		m_init = 0;
	}
}

int TVoctreeVLFeat::init_read(const char *fname, int truncate){
	return this->read_hikm(fname);
}

int TVoctreeVLFeat::init(const uchar *data , int DataSize, int TreeMode, int numOfDimension){	

	switch (TreeMode)
	{
	case VLFeat_Init_Mode::Small_Tree:
		m_depth = 4;
		m_K = 10;
		break;
	case VLFeat_Init_Mode::Middle_Tree:
		m_depth = 6;
		m_K = 10;
		break;
	case VLFeat_Init_Mode::Large_Tree:
		m_depth = 8;
		m_K = 7;
		break;
	case VLFeat_Init_Mode::TEST_Tree:
		m_depth = 3;
		m_K = 5;
		break;
	default:
		m_depth = 6;
		m_K = 10;
		break;
	}
		
	m_M = numOfDimension; 
	m_max_niters = 50;
	m_method = VlIKMAlgorithms::VL_IKM_ELKAN;
	m_verb = 5;	

	vocabTree.depth = m_depth;
	vocabTree.K = m_K;
	vocabTree.M = m_M;
	vocabTree.max_niters = m_max_niters;
	vocabTree.method = m_method;
	vocabTree.verb = m_verb;

	VL_PRINTF("Voc Tree Train Started Depth: %d, Branch: %d \n", m_depth, m_K) ;
	vl_tic();
	vl_size dataSize = vl_size(DataSize);
	vl_size nThr = vl_get_max_threads();
	vl_set_num_threads(nThr);
	vl_hikm_train(&vocabTree,data,vl_size(dataSize));	
	VL_PRINTF("Voc Tree Train Time: %.3f s\n", vl_toc ()) ;	
	
	m_init = 1;
	return 1;
}

void TVoctreeVLFeat::clean(void) {
	if (m_init == 1) {
		//delete [] m_depth;
		//delete [] m_K;
		//delete [] m_M;
		//delete [] m_max_niters;
		//delete [] m_method;
		//delete [] m_root;
		//delete [] m_verb;
		m_init = 0;
	}
}

int TVoctreeVLFeat::ReturnMaxNodeNumber(vl_size depth , vl_size K ) const
{
	
	if( depth>2)
	{
		return pow(double(K), double(depth)-1) + ReturnMaxNodeNumber(depth-2,K);
	}
	else
		return K+1;
};

int TVoctreeVLFeat::ReturnMaxNodeNumber() const
{
	vl_size depth = m_depth;
	vl_size K = m_K;
	
	if( depth>2)
	{
		return pow(double(K), double(depth)-1) + ReturnMaxNodeNumber(depth-2,K);
	}
	else
		return K+1;
};

void TVoctreeVLFeat::dump_node(VlHIKMNode* node,int *c, FILE* f)
{
	VlIKMFilt* filter = node->filter;
	fwrite(&filter->K, sizeof(vl_size), 1, f);
	fwrite(filter->centers, sizeof(vl_ikmacc_t), m_M*filter->K, f);
	fwrite(filter->inter_dist, sizeof(vl_ikmacc_t), filter->K*filter->K, f);
	m_stupidCount ++;
}

void TVoctreeVLFeat::dump_tree(VlHIKMNode* root, int *c, FILE* f)
{
	if (root == nullptr) 
		return;

	dump_node(root,c, f);

	if (root->children)
	{
		for (int i=0; i<root->filter->K; i++)
			dump_tree(root->children[i],c, f);
	}
}

int TVoctreeVLFeat::write_hikm(const char* FileName)
{
	VlHIKMNode* root = this->vocabTree.root;

	FILE* f = new FILE;
	fopen_s(&f,FileName, "wb");

	if (!f)
	{
		fprintf(stderr, "Cannot open file %s\n", FileName);
		return 0;
	}

	vl_size magic = VL_FEAT_HIKM_TREE_MAGIC;
	fwrite(&magic, sizeof(vl_size), 1, f);
	fwrite(&vocabTree.depth, sizeof(vl_size), 1, f);
	fwrite(&vocabTree.M, sizeof(vl_size), 1, f);
	fwrite(&vocabTree.K, sizeof(vl_size), 1, f);
	fwrite(&vocabTree.max_niters, sizeof(vl_size), 1, f);
	fwrite(&vocabTree.method, sizeof(int), 1, f);
	fwrite(&vocabTree.verb, sizeof(int), 1, f);

	dump_tree(root, nullptr, f);
	fclose(f);
}

void TVoctreeVLFeat::load_node(VlHIKMNode* node, int *c, FILE* f)
{
	for (int i=0; i<m_K; i++)
	{
		node->children[i] = create_tree_node(f);
		//m_stupidCount ++;
	}
}

void TVoctreeVLFeat::load_tree(VlHIKMNode** root, int c, FILE* f)
{
	VlHIKMNode* node = nullptr;
	if (c < 0) 
		return;

	*root = create_tree_node(f);
	//m_stupidCount++;

	for (int i=0; i<m_K; i++)
		load_tree(&(*root)->children[i], c-1, f);		
}

VlHIKMNode *TVoctreeVLFeat::create_tree_node(FILE* f)
{
	//vl_ikmacc_t* centers, vl_ikmacc_t* inter_dist
	VlHIKMNode *node = (VlHIKMNode*)vl_malloc (sizeof(VlHIKMNode)) ;
	node->filter = vl_ikm_new(m_method);
	node->filter->max_niters = m_max_niters;
	node->filter->verb = m_verb;
	node->filter->M = m_M;
	
	fread(&node->filter->K, sizeof(vl_size), 1, f);
	node->filter->centers = (vl_ikmacc_t*)vl_calloc(m_M * node->filter->K, sizeof(vl_ikmacc_t));
	node->filter->inter_dist = (vl_ikmacc_t*)vl_calloc(node->filter->K*node->filter->K, sizeof(vl_ikmacc_t));

	fread(node->filter->centers, sizeof(vl_ikmacc_t), m_M*node->filter->K, f);
	fread(node->filter->inter_dist, sizeof(vl_ikmacc_t), node->filter->K*node->filter->K, f);

	node->children = (VlHIKMNode**)vl_calloc (m_K, sizeof(*node->children)) ;
	//m_stupidCount ++;
	return node;
}

int TVoctreeVLFeat::read_hikm(const char* FileName)
{
	FILE* f = new FILE;
	fopen_s(&f, FileName, "rb");

	if (!f)
	{
		fprintf(stderr, "Cannot open file %s\n", FileName);
		return 0;
	}

	vl_size magic = 0;
	long long temp = 0;
	long long tempi= 0;
	fread(&magic, sizeof(vl_size), 1, f);

	if (magic!=VL_FEAT_HIKM_TREE_MAGIC)
	{
		fprintf(stderr, "The file is not a valid HIKM tree\n");
		return 0;
	}

	fread(&temp, sizeof(vl_size), 1, f);
	m_depth = temp;
	fread(&temp, sizeof(vl_size), 1, f);
	m_M = temp;
	fread(&temp, sizeof(vl_size), 1, f);
	m_K = temp;
	fread(&temp, sizeof(vl_size), 1, f);
	this->m_max_niters = temp;
	fread(&tempi, sizeof(int), 1, f);
	this->m_method = tempi;
	fread(&tempi, sizeof(int), 1, f);
	this->m_verb = tempi;

	//this->vocabTree = vl_hikm_new(m_method);
	this->vocabTree.method = m_method;
	this->vocabTree.root = nullptr;
	this->vocabTree.max_niters = m_max_niters;
	vl_hikm_init(&this->vocabTree, m_M, m_K, m_depth);
	
	m_centerTemp = new vl_ikmacc_t[m_M*m_K];
	m_interDist = new vl_ikmacc_t[m_K*m_K];
	
	//m_stupidCount ++;
	load_tree(&this->vocabTree.root, m_depth-1, f);
	fclose(f);

	delete[] m_centerTemp;
	delete[] m_interDist;
	return 1;
}

void TVoctreeVLFeat::quantize(unsigned int *vwi, unsigned char *sift){
	vl_uint32 *asgn = new vl_uint32[int(m_depth)]();
	vl_hikm_push(&vocabTree, asgn,sift,vl_size(1));
	*vwi=0;
	//printf("Full path : ");
	for(int i=0; i<int(m_depth); i++)
	{
		*vwi = *vwi*int(m_K) +asgn[i];
		//printf("%d ", asgn[i]);
		//*vwi |= ((asgn[i]<<i))); // supports depth<=10, branch<=9
		//*vwi |= ((asgn[i]<<((int)m_depth-i-1))); // supports depth<=10, branch<=9		
	}
	//printf("\n");
}

void TVoctreeVLFeat::quantize_multi(unsigned int *vwM, unsigned char *sift, int numDesc, int dimOfDesc) 
{
#if defined _OPENMP
#pragma omp parallel for
#endif
	for (int k = 0; k < numDesc; k++) {
		this->quantize(&vwM[k], &sift[k*dimOfDesc]);
	}
}

