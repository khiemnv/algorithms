#include <stdio.h>
#include <string.h>
#include <crtdbg.h>
#include <Windows.h>
#include <intrin.h>

//test data
typedef struct pair {
	int key;
	const char* zword;
}PAIR;

PAIR g_data[] = {
{1, "From"},
{2, "Mexico"},
{3, "to"},
{4, "Argentina"},
{5, "Latin"},
{6, "American"},
{7, "governments"},
{8, "are"},
{9, "watching"},
{10, "the"},
{11, "US"},
{12, "election"},
{13, "race"},
{14, "closely"},
{15, "calculating"},
{16, "how"},
{17, "a"},
{18, "Trump"},
{19, "or"},
{20, "Biden"},
{21, "win"},
{22, "will"},
{23, "affect"},
{24, "their"},
{25, "ties"},
{26, "with"},
{27, "superpower"},
{28, "Left-wing-ruled"},
{29, "nations"},
{30, "such"},
{31, "as"},
{32, "Cuba"},
{33, "Venezuela"},
{34, "and"},
{35, "Nicaragua"},
{36, "which"},
{37, "targets"},
{38, "of"},
{39, "President"},
{40, "Donald"},
{41, "Trump's"},
{42, "incendiary"},
{43, "rhetoric"},
{44, "ramped"},
{45, "up"},
{46, "sanctions"},
{47, "dearly"},
{48, "hoping"},
{49, "that"},
{50, "Republican"},
{51, "fails"},
{52, "in"},
{53, "his"},
{54, "re-election"},
{55, "bid"},
{56, "They"},
{57, "perceive"},
{58, "Democratic"},
{59, "challenger"},
{60, "Joe"},
{61, "may"},
{62, "offer"},
{63, "less"},
{64, "hostility"},
{65, "even"},
{66, "possible"},
{67, "negotiations"},
{68, "Some"},
{69, "leaders"},
{70, "Brazil's"},
{71, "far-right"},
{72, "Jair"},
{73, "Bolsonaro"},
{74, "Colombia's"},
{75, "Ivan"},
{76, "Duque"},
{77, "have"},
{78, "forged"},
{79, "friendly"},
{80, "relations"},
{81, "watch"},
{82, "opinion"},
{83, "polls"},
{84, "be"},
{85, "concerned"},
{86, "could"},
{87, "bring"},
{88, "more"},
{89, "scrutiny"},
{90, "on"},
{91, "environment"},
{92, "human"},
{93, "rights"},
{94, "most"},
{95, "immediate"},
{96, "neighbour"},
{97, "left-leaning"},
{98, "Mexican"},
{99, "Andrés"},
{100, "Manuel"},
{101, "López"},
{102, "Obrador"},
{103, "has"},
{104, "bent"},
{105, "over"},
{106, "backwards"},
{107, "maintain"},
{108, "cordial"},
{109, "relationship"},
{110, "tolerating"},
{111, "public"},
{112, "insults"},
{113, "about"},
{114, "Mexicans"},
{115, "repeated"},
{116, "threats"},
{117, "For"},
{118, "AMLO"},
{119, "is"},
{120, "popularly"},
{121, "known"},
{122, "things"},
{123, "can"},
{124, "only"},
{125, "get"},
{126, "better"},
{127, "victory"},
{128, "Argentina's"},
{129, "Peronist"},
{130, "Alberto"},
{131, "Fernández"},
{132, "also"},
{133, "been"},
{134, "careful"},
{135, "not"},
{136, "pick"},
{137, "any"},
{138, "serious"},
{139, "fights"},
{140, "But"},
{141, "he"},
{142, "would"},
{143, "no"},
{144, "doubt"},
{145, "prefer"},
{146, "see"},
{147, "White"},
{148, "House"},
{149, "harmonious"},
{150, "US-China"},
{151, "under"},
{152, "favour"},
{153, "current"},
{154, "Argentine"},
{155, "outreach"},
{156, "Asian"},
{157, "power"},
{158, "Skip"},
{159, "main"},
{160, "content"},
{161, "Search"},
{162, "Sign"},
{163, "Windows"},
{164, "Apps"},
{165, "Win32"},
{166, "Desktop"},
{167, "Technologies"},
{168, "Diagnostics"},
{169, "Performance"},
{170, "Counters"},
{171, "Filter"},
{172, "by"},
{173, "title"},
{174, "What"},
{175, "s"},
{176, "New"},
{177, "Reference"},
{178, "Data"},
{179, "Types"},
{180, "Helper"},
{181, "Error"},
{182, "Codes"},
{183, "5"},
{184, "31"},
{185, "2018"},
{186, "2"},
{187, "minutes"},
{188, "read"},
{189, "PDH"},
{190, "Consumers"},
{191, "use"},
{192, "functions"},
{193, "following"},
{194, "type"},
{195, "Description"},
{196, "PDH_HQUERY"},
{197, "Handle"},
{198, "query"},
{199, "PdhOpenQuery"},
{200, "function"},
{201, "returns"},
{202, "this"},
{203, "Close"},
{204, "using"},
{205, "PdhCloseQuery"},
{206, "PDH_HCOUNTER"},
{207, "counter"},
{208, "PdhAddCounter"},
{209, "PdhRemoveCounter"},
{210, "closing"},
{211, "contains"},
{212, "Do"},
{213, "call"},
{214, "if"},
{215, "corresponding"},
{216, "already"},
{217, "closed"},
{218, "maintains"},
{219, "linkage"},
{220, "between"},
{221, "queries"},
{222, "automatically"},
{223, "handles"},
{224, "when"},
{225, "PDH_HLOG"},
{226, "log"},
{227, "file"},
{228, "PdhOpenLog"},
{229, "PdhBindInputDataSource"},
{230, "return"},
{231, "PdhCloseLog"},
{232, "PDH_STATUS"},
{233, "status"},
{234, "value"},
{235, "All"},
{236, "succeeds"},
{237, "ERROR_SUCCESS"},
{238, "Otherwise"},
{239, "system"},
{240, "code"},
{241, "page"},
{242, "helpful"},
{243, "Yes"},
{244, "article"},
{245, "English"},
{246, "United"},
{247, "States"},
{248, "Previous"},
{249, "Version"},
{250, "Docs"},
{251, "Blog"},
{252, "Contribute"},
{253, "Privacy"},
{254, "Cookies"},
{255, "Terms"},
{256, "Site"},
{257, "Feedback"},
{258, "Trademarks"},
{259, "Microsoft"},
{260, "2020"},
{261, "Tools"},
{262, "CTRPP"},
{263, "8"},
{264, "17"},
{265, "7"},
{266, "1"},
{267, "tool"},
{268, "pre"},
{269, "processor"},
{270, "parses"},
{271, "validates"},
{272, "manifest"},
{273, "your"},
{274, "V2"},
{275, "provider"},
{276, "generates"},
{277, "rc"},
{278, "resources"},
{279, "strings"},
{280, "needed"},
{281, "it"},
{282, "h"},
{283, "header"},
{284, "you"},
{285, "provide"},
{286, "should"},
{287, "run"},
{288, "during"},
{289, "build"},
{290, "generated"},
{291, "starting"},
{292, "point"},
{293, "developing"},
{294, "instead"},
{295, "trying"},
{296, "generate"},
{297, "yourself"},
{298, "syntax"},
{299, "Copy"},
{300, "o"},
{301, "codeFile"},
{302, "rcFile"},
{303, "legacy"},
{304, "MemoryRoutines"},
{305, "NotificationCallback"},
{306, "prefix"},
{307, "ch"},
{308, "symFile"},
{309, "backcompat"},
{310, "inputFile"},
{311, "Arguments"},
{312, "Option"},
{313, "Required"},
{314, "Specifies"},
{315, "name"},
{316, "man"},
{317, "XML"},
{318, "defines"},
{319, "contain"},
{320, "C"},
{321, "inline"},
{322, "simplify"},
{323, "initializing"},
{324, "uninitializing"},
{325, "resource"},
{326, "string"},
{327, "table"},
{328, "optional"},
{329, "symbol"},
{330, "symbols"},
{331, "names"},
{332, "GUIDs"},
{333, "each"},
{334, "counterset"},
{335, "variables"},
{336, "defined"},
{337, "Changes"},
{338, "default"},
{339, "signature"},
{340, "CounterInitialize"},
{341, "include"},
{342, "parameters"},
{343, "specifying"},
{344, "ControlCallback"},
{345, "AllocateMemory"},
{346, "FreeMemory"},
{347, "callback"},
{348, "argument"},
{349, "same"},
{350, "effect"},
{351, "including"},
{352, "attribute"},
{353, "element"},
{354, "migrate"},
{355, "outputFile"},
{356, "generating"},
{357, "files"},
{358, "upgrades"},
{359, "latest"},
{360, "saves"},
{361, "switch"},
{362, "cannot"},
{363, "used"},
{364, "other"},
{365, "switches"},
{366, "Usage"},
{367, "NewFile"},
{368, "OldFile"},
{369, "Deprecated"},
{370, "Support"},
{371, "kernel"},
{372, "mode"},
{373, "providers"},
{374, "was"},
{375, "added"},
{376, "incompatible"},
{377, "earlier"},
{378, "versions"},
{379, "driver"},
{380, "fail"},
{381, "load"},
{382, "due"},
{383, "missing"},
{384, "Pcw"},
{385, "APIs"},
{386, "Set"},
{387, "enable"},
{388, "compatibility"},
{389, "dynamically"},
{390, "necessary"},
{391, "silently"},
{392, "disable"},
{393, "available"},
{394, "includes"},
{395, "templates"},
{396, "memory"},
{397, "routines"},
{398, "_r"},
{399, "Vista"},
{400, "PerfAutoInitialize"},
{401, "PerfAutoCleanup"},
{402, "CounterCleanup"},
{403, "named"},
{404, "based"},
{405, "written"},
{406, "directory"},
{407, "contained"},
{408, "Remarks"},
{409, "an"},
{410, "optionally"},
{411, "localizable"},
{412, "countersets"},
{413, "Important"},
{414, "must"},
{415, "included"},
{416, "into"},
{417, "binary"},
{418, "full"},
{419, "path"},
{420, "registered"},
{421, "installation"},
{422, "unable"},
{423, "locate"},
{424, "handled"},
{425, "follows"},
{426, "developer"},
{427, "edits"},
{428, "applicationIdentity"},
{429, "DLL"},
{430, "SYS"},
{431, "EXE"},
{432, "installed"},
{433, "part"},
{434, "component"},
{435, "reads"},
{436, "compiler"},
{437, "compiles"},
{438, "res"},
{439, "containing"},
{440, "done"},
{441, "either"},
{442, "directly"},
{443, "compiling"},
{444, "another"},
{445, "via"},
{446, "directive"},
{447, "linker"},
{448, "embeds"},
{449, "copied"},
{450, "onto"},
{451, "user"},
{452, "lodctr"},
{453, "converts"},
{454, "records"},
{455, "registry"},
{456, "m"},
{457, "combine"},
{458, "specified"},
{459, "form"},
{460, "diagnostic"},
{461, "purposes"},
{462, "inspect"},
{463, "recorded"},
{464, "checking"},
{465, "key"},
{466, "HKEY_LOCAL_MACHINE"},
{467, "SOFTWARE"},
{468, "NT"},
{469, "CurrentVersion"},
{470, "Perflib"},
{471, "_V2Providers"},
{472, "ProviderGuid"},
{473, "MUI"},
{474, "localization"},
{475, "sure"},
{476, "along"},
{477, "collection"},
{478, "consumer"},
{479, "uses"},
{480, "providerType"},
{481, "userMode"},
{482, "definitions"},
{483, "coding"},
{484, "initialization"},
{485, "prefixCounterInitialize"},
{486, "cleanup"},
{487, "prefixCounterCleanup"},
{488, "global"},
{489, "variable"},
{490, "stores"},
{491, "opened"},
{492, "calls"},
{493, "PerfCreateInstance"},
{494, "PerfDeleteInstance"},
{495, "controlling"},
{496, "countersetGUID"},
{497, "GUID"},
{498, "plus"},
{499, "suffix"},
{500, "e"},
{501, "g"},
{502, "MyCounterSetGUID"},
{503, "macro"},
{504, "id"},
{505, "PerfSetCounterRefValue"},
{506, "PerfSetULongLongCounterValue"},
{507, "setting"},
{508, "refers"},
{509, "command"},
{510, "line"},
{511, "parameter"},
{512, "kernelMode"},
{513, "prefixRegisterCounterset"},
{514, "fills"},
{515, "RegInfo"},
{516, "structure"},
{517, "then"},
{518, "invokes"},
{519, "PcwRegister"},
{520, "putting"},
{521, "resulting"},
{522, "registration"},
{523, "prefixUnregisterCounterset"},
{524, "PcwUnregister"},
{525, "instance"},
{526, "creation"},
{527, "prefixCreateCounterset"},
{528, "array"},
{529, "PcwData"},
{530, "structures"},
{531, "PcwCreateInstance"},
{532, "prefixCloseCounterset"},
{533, "PcwCloseInstance"},
{534, "reporting"},
{535, "prefixAddCounterset"},
{536, "PcwAddInstance"},
{537, "SDK"},
{538, "20H1"},
{539, "later"},
{540, "prefixInitRegistrationInformationCounterset"},
{541, "advanced"},
{542, "scenarios"},
{543, "cases"},
{544, "where"},
{545, "does"},
{546, "meet"},
{547, "needs"},
{548, "want"},
{549, "customize"},
{550, "values"},
{551, "store"},
{552, "returned"},
{553, "Note"},
{554, "writing"},
{555, "programs"},
{556, "hard"},
{557, "coded"},
{558, "consume"},
{559, "Requirements"},
{560, "Minimum"},
{561, "supported"},
{562, "client"},
{563, "server"},
{564, "2008"},

};

class dictIF
{
public:
	virtual int add(char*, int) { return 0; };
	virtual int get(char*) { return 0; };
};

//use pointer
#define max_count 1024*1024
class dict1:public dictIF
{
private:
	typedef struct node
	{
		node* psib;
		node* pchild;
		char key;
		int value;
	}NODE;

	NODE *m_data;
	int m_count;

	NODE* crtNode(char ch) {
		_ASSERT(m_count < max_count);
		NODE* pNode = &m_data[m_count];
		m_count++;
		pNode->key = ch;
		pNode->value = -1;
		pNode->psib = nullptr;
		pNode->pchild = nullptr;
		return pNode;
	}
	NODE* m_root;
public:
	dict1() {
		m_data = (NODE*)malloc(sizeof(NODE) * max_count);
		m_count = 0;
		m_root = crtNode('a');
	}
	~dict1() {
		free(m_data);
	}
	int add(char* zword, int value) {
		NODE* pNode = m_root;
		int sts = 0;
		for (int i = 0; i < strlen(zword); i++) {
			switch (sts)
			{
			case 0:
				//find in lvl[i]
 				for (; pNode != nullptr;) {
					if (pNode->key == zword[i]) {
						//found
						if (pNode->pchild == nullptr) {
							sts = 1;
						}
						else if (i == strlen(zword) - 1) {
							break;
						} 
						else {
							pNode = pNode->pchild;
						}
						break;
					}
					else if (pNode->psib == nullptr) {
						//new branch
						pNode->psib = crtNode(zword[i]);
						pNode = pNode->psib;
						sts = 2;
						break;
					}
					else
					{
						//find next
						pNode = pNode->psib;
					}
				}
				break;
			case 1:
				//go deeper
				pNode->pchild = crtNode(zword[i]);
				pNode = pNode->pchild;
				break;
			case 2:
				//new branch
				pNode->pchild = crtNode(zword[i]);
				pNode = pNode->pchild;
				break;
			}
		}
		_ASSERT(pNode->value == -1);
		pNode->value = value;
		return 0;
	}
	int get(char* zword) {
		NODE* pNode = m_root;
		for (int i = 0; ; i++) {

			//find in lvl[i]
			for (; pNode != nullptr;) {
				if (pNode->key == zword[i]) {
					//found
					break;
				}
				else
				{
					//find next
					pNode = pNode->psib;
				}
			}

			if (pNode == nullptr) {
				return -1;
			}
			if (i == strlen(zword) - 1) break;

			pNode = pNode->pchild;
		}
		return pNode->value;
	};
};


//use index
#define getNode(idx) (m_data[idx])
#define getNodeSib(idx) (m_data[idx].psib)
#define getNodeChild(idx) (m_data[idx].pchild)
#define getNodeKey(idx) (m_data[idx].key)
#define getNodeValue(idx) (m_data[idx].value)
class dict2:public dictIF
{
private:
	typedef struct node
	{
		int psib;
		int pchild;
		char key;
		int value;
	}NODE;

	NODE* m_data;
	int m_count;

	int crtNode(char ch) {
		int newidx = m_count;
		_ASSERT(m_count < max_count);
		m_count++;
		getNode(newidx).key = ch;
		getNode(newidx).value = -1;
		getNode(newidx).psib = -1;
		getNode(newidx).pchild = -1;
		return newidx;
	}
	int m_root;
public:
	dict2() {
		m_data = (NODE*)malloc(sizeof(NODE) * max_count);
		m_count = 0;
		m_root = crtNode('a');
	}
	~dict2() {
		free(m_data);
	}
	int add(char* zword, int value) {
		int iNode = m_root;
		int sts = 0;
		for (int i = 0; i < strlen(zword); i++) {
			switch (sts)
			{
			case 0:
				//find in lvl[i]
				for (; iNode != -1;) {
					if (getNode(iNode).key == zword[i]) {
						//found
						if (getNode(iNode).pchild == -1) {
							sts = 1;
						}
						else if (i == strlen(zword) - 1) {
							break;
						}
						else {
							iNode = getNode(iNode).pchild;
						}
						break;
					}
					else if (getNode(iNode).psib == -1) {
						//new branch
						getNode(iNode).psib = crtNode(zword[i]);
						iNode = getNode(iNode).psib;
						sts = 2;
						break;
					}
					else
					{
						//find next
						iNode = getNode(iNode).psib;
					}
				}
				break;
			case 1:
				//go deeper
				getNode(iNode).pchild = crtNode(zword[i]);
				iNode = getNode(iNode).pchild;
				break;
			case 2:
				//new branch
				getNode(iNode).pchild = crtNode(zword[i]);
				iNode = getNode(iNode).pchild;
				break;
			}
		}

		getNode(iNode).value = value;
		return 0;
	}
	int get(char* zword) {
		int iNode = m_root;
		for (int i = 0; ; i++) {

			//find in lvl[i]
			for (; iNode != -1;) {
				if (getNode(iNode).key == zword[i]) {
					//found
					break;
				}
				else
				{
					//find next
					iNode = getNode(iNode).psib;
				}
			}

			if (iNode == -1) {
				return -1;
			}
			if (i == strlen(zword) - 1) break;

			iNode = getNode(iNode).pchild;
		}
		return getNode(iNode).value;
	};
};


void test1(dictIF* obj)
{
	int n = sizeof(g_data) / sizeof(g_data[0]);
	for (int i = 0; i < n; i++) {
		obj->add((char*)g_data[i].zword, g_data[i].key);
	}
	for (int i = 0; i < n; i++) {
		_ASSERT(obj->get((char*)g_data[i].zword) == g_data[i].key);
	}
}

DWORD64 getClk() {
	return __rdtsc();
}

int main()
{
	dict1 obj;
	dict2 obj2;

	DWORD64 start;
	DWORD64 end;
	DWORD64 start2;
	DWORD64 end2;

	start2 = getClk();
	obj2.add((char*)"mot", 1);
	obj2.add((char*)"hai", 2);
	obj2.add((char*)"ba", 3);
	obj2.add((char*)"bon", 4);
	obj2.add((char*)"nam", 5);
	obj2.add((char*)"sau", 6);
	obj2.add((char*)"bay", 7);
	obj2.add((char*)"tam", 8);
	obj2.add((char*)"chin", 9);

	_ASSERT(obj2.get((char*)"chin") == 9);
	_ASSERT(obj2.get((char*)"tam") == 8);
	_ASSERT(obj2.get((char*)"bay") == 7);
	_ASSERT(obj2.get((char*)"bon") == 4);
	_ASSERT(obj2.get((char*)"ba") == 3);
	test1(&obj2);
	end2 = getClk();

	start = getClk();
	obj.add((char*)"mot", 1);
	obj.add((char*)"hai", 2);
	obj.add((char*)"ba", 3);
	obj.add((char*)"bon", 4);
	obj.add((char*)"nam", 5);
	obj.add((char*)"sau", 6);
	obj.add((char*)"bay", 7);
	obj.add((char*)"tam", 8);
	obj.add((char*)"chin", 9);

	_ASSERT(obj.get((char*)"chin") == 9);
	_ASSERT(obj.get((char*)"tam") == 8);
	_ASSERT(obj.get((char*)"bay") == 7);
	_ASSERT(obj.get((char*)"bon")== 4);
	_ASSERT(obj.get((char*)"ba")== 3);

	test1(&obj);
	end = getClk();


	printf("dic1 %I64d\n", end - start);
	printf("dic2 %I64d\n", end2 - start2);

	
	return 0;
}