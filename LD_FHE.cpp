#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <thread>
#include <mutex>
#include <memory>
#include <limits>
#include <complex>
#include <iterator>

#include "seal/seal.h"

using namespace std;
using namespace seal;

/*
Helper function: Prints the name of the example in a fancy banner.
*/
void print_example_banner(string title)
{
    if (!title.empty())
    {
        size_t title_length = title.length();
        size_t banner_length = title_length + 2 + 2 * 10;
        string banner_top(banner_length, '*');
        string banner_middle = string(10, '*') + " " + title + " " + string(10, '*');

        cout << endl
            << banner_top << endl
            << banner_middle << endl
            << banner_top << endl
            << endl;
    }
}

/*
Helper function: Prints the parameters in a SEALContext.
*/
void print_parameters(const shared_ptr<SEALContext> &context)
{
    auto &context_data = *context->context_data();

    /*
    Which scheme are we using?
    */
    string scheme_name;
    switch (context_data.parms().scheme())
    {
    case scheme_type::BFV:
        scheme_name = "BFV";
        break;
    case scheme_type::CKKS:
        scheme_name = "CKKS";
        break;
    default:
        throw invalid_argument("unsupported scheme");
    }

    cout << "/ Encryption parameters:" << endl;
    cout << "| scheme: " << scheme_name << endl;
    cout << "| poly_modulus_degree: " << 
        context_data.parms().poly_modulus_degree() << endl;

    /*
    Print the size of the true (product) coefficient modulus.
    */
    cout << "| coeff_modulus size: " << context_data.
        total_coeff_modulus().significant_bit_count() << " bits" << endl;

    /*
    For the BFV scheme print the plain_modulus parameter.
    */
    if (context_data.parms().scheme() == scheme_type::BFV)
    {
        cout << "| plain_modulus: " << context_data.
            parms().plain_modulus().value() << endl;
    }

    cout << "\\ noise_standard_deviation: " << context_data.
        parms().noise_standard_deviation() << endl;
    cout << endl;
}

/*
Helper function: Prints the `parms_id' to std::ostream.
*/
ostream &operator <<(ostream &stream, parms_id_type parms_id)
{
    stream << hex << parms_id[0] << " " << parms_id[1] << " "
        << parms_id[2] << " " << parms_id[3] << dec;
    return stream;
}

/*
Helper function: Prints a vector of floating-point values.
*/
template<typename T>
void print_vector(vector<T> vec, size_t print_size = 4, int prec = 3)
{
    /*
    Save the formatting information for std::cout.
    */
    ios old_fmt(nullptr);
    old_fmt.copyfmt(cout);

    size_t slot_count = vec.size();

    cout << fixed << setprecision(prec) << endl;
    if(slot_count <= 2 * print_size)
    {
        cout << "    [";
        for (size_t i = 0; i < slot_count; i++)
        {
            cout << " " << vec[i] << ((i != slot_count - 1) ? "," : " ]\n");
        }
    }
    else
    {
        vec.resize(max(vec.size(), 2 * print_size));
        cout << "    [";
        for (size_t i = 0; i < print_size; i++)
        {
            cout << " " << vec[i] << ",";
        }
        if(vec.size() > 2 * print_size)
        {
            cout << " ...,";
        }
        for (size_t i = slot_count - print_size; i < slot_count; i++)
        {
            cout << " " << vec[i] << ((i != slot_count - 1) ? "," : " ]\n");
        }
    }
    cout << endl;

    /*
    Restore the old std::cout formatting.
    */
    cout.copyfmt(old_fmt);
}

void LD(int, int);
void LD_P();

int main()
{
#ifdef SEAL_VERSION
    cout << "SEAL version: " << SEAL_VERSION << endl;
#endif
    while (true)
    {
        cout << "\nSEAL:" << endl << endl;
        cout << " 1. LD Test" << endl;
		cout << " 2. LD Test (Optimized)" << endl;
        cout << " 0. Exit" << endl;

        /*
        Print how much memory we have allocated from the current memory pool.
        By default the memory pool will be a static global pool and the
        MemoryManager class can be used to change it. Most users should have
        little or no reason to touch the memory allocation system.
        */
        cout << "\nTotal memory allocated from the current memory pool: "
            << (MemoryManager::GetPool().alloc_byte_count() >> 20) << " MB" << endl;

        int selection = 0;
        cout << endl << "Run: ";
        if (!(cin >> selection))
        {
            cout << "Invalid option." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        
        switch (selection)
        {
        case 1:
			int s, ps;
			cout << endl << "Size: ";
			cin >> s;
			cout << endl << "Plain_Size: ";
			cin >> ps;
            LD(s,ps);
            break;
        
        case 2:
                LD_P();
                break;
                
        case 0:
            return 0;

        default:
            cout << "Invalid option." << endl;
        }
    }

    return 0;
}


void LD(int size, int plain_size)
{
	print_example_banner("LD Test");

	// set all parameters of SEAL
	EncryptionParameters parms(scheme_type::BFV);
	parms.set_poly_modulus_degree(4096);
	parms.set_coeff_modulus(coeff_modulus_128(4096));
	parms.set_plain_modulus(1 << plain_size);

	auto context = SEALContext::Create(parms);
	print_parameters(context);

	IntegerEncoder encoder(parms.plain_modulus());

	// generate the public and secret keys
	KeyGenerator keygen(context);
	auto public_key = keygen.public_key();
	auto secret_key = keygen.secret_key();

	// set up an Encryptor, Evaluator, and Decryptor
	Encryptor encryptor(context, public_key);
	Evaluator evaluator(context);
	Decryptor decryptor(context, secret_key);

	// some random inputs
	int A = 10;
	Plaintext plain_A = encoder.encode(A);
	Ciphertext encrypted_A;
	encryptor.encrypt(plain_A, encrypted_A);

	int B = 15;
	Plaintext plain_B = encoder.encode(B);
	Ciphertext encrypted_B;
	encryptor.encrypt(plain_B, encrypted_B);

	int C = 123;
	Plaintext plain_C = encoder.encode(C);
	Ciphertext encrypted_C;
	encryptor.encrypt(plain_C, encrypted_C);

	int D = 76;
	Plaintext plain_D = encoder.encode(D);
	Ciphertext encrypted_D;
	encryptor.encrypt(plain_D, encrypted_D);

	// all the ciphers
	Ciphertext NA, Na, NB, Nb, NANB, NaNb, NANaNBNb, NNAB, R, Result;

	// timing
	chrono::high_resolution_clock::time_point time_start, time_end;

	// value N
	int N = 64;
	Plaintext plain_N = encoder.encode(N);

	// threshold
	int T = 11;
	Plaintext plain_T = encoder.encode(T);

	// key for relinearization
	auto relin_keys = keygen.relin_keys(30);

	time_start = chrono::high_resolution_clock::now();
	for (int i = 0; i < size; i++)
	{
		// four additions
		evaluator.add(encrypted_A, encrypted_B, NA);
		evaluator.add(encrypted_C, encrypted_D, Na);
		evaluator.add(encrypted_A, encrypted_C, NB);
		evaluator.add(encrypted_B, encrypted_D, Nb);

		// three multiplications
		evaluator.multiply(NA, NB, NANB);
		evaluator.relinearize_inplace(NANB, relin_keys);

		evaluator.multiply(Na, Nb, NaNb);
		evaluator.relinearize_inplace(NaNb, relin_keys);

		evaluator.multiply(NANB, NaNb, NANaNBNb);
		evaluator.relinearize_inplace(NANaNBNb, relin_keys);

		// two scalar multiplications
		evaluator.multiply_plain(encrypted_A, plain_N, NNAB);
		evaluator.multiply_plain(NANaNBNb, plain_T, R);

		// one negation
		evaluator.negate_inplace(NANB);

		// one addition
		evaluator.add_inplace(NNAB, NANB);

		// one squar
		evaluator.square_inplace(NNAB);
		evaluator.relinearize_inplace(NNAB, relin_keys);

		// one negation
		evaluator.negate_inplace(R);

		// one addition
		evaluator.add(NNAB, R, Result);
	}
	time_end = chrono::high_resolution_clock::now();
	auto time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);
	cout << "Done [" << time_diff.count() << " microseconds]" << endl;
}

void LD_P()
{
    print_example_banner("LD Test (Optimized)");
    
    
    EncryptionParameters parms(scheme_type::BFV);
    parms.set_poly_modulus_degree(4096);
    parms.set_coeff_modulus(coeff_modulus_128(4096));
    
    parms.set_plain_modulus(40961);
    
    auto context = SEALContext::Create(parms);
    print_parameters(context);
    
    auto qualifiers = context->context_data()->qualifiers();
    cout << "Batching enabled: " << boolalpha << qualifiers.using_batching << endl;
    
    KeyGenerator keygen(context);
    auto public_key = keygen.public_key();
    auto secret_key = keygen.secret_key();
    
    auto relin_keys = keygen.relin_keys(30);
    
    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);
    
    BatchEncoder batch_encoder(context);
    
    size_t slot_count = batch_encoder.slot_count();
    size_t row_size = slot_count / 2;
    cout << "Plaintext matrix row size: " << row_size << endl;
    
    vector<uint64_t> A(slot_count, 7ULL);
    Plaintext plain_A;
    batch_encoder.encode(A, plain_A);
    Ciphertext encrypted_A;
    encryptor.encrypt(plain_A, encrypted_A);
    
    vector<uint64_t> B(slot_count, 6ULL);
    Plaintext plain_B;
    batch_encoder.encode(B, plain_B);
    Ciphertext encrypted_B;
    encryptor.encrypt(plain_B, encrypted_B);
    
    vector<uint64_t> C(slot_count, 7ULL);
    Plaintext plain_C;
    batch_encoder.encode(C, plain_C);
    Ciphertext encrypted_C;
    encryptor.encrypt(plain_C, encrypted_C);
    
    vector<uint64_t> D(slot_count, 7ULL);
    Plaintext plain_D;
    batch_encoder.encode(D, plain_D);
    Ciphertext encrypted_D;
    encryptor.encrypt(plain_D, encrypted_D);
    
    // all the ciphers
    Ciphertext NA, Na, NB, Nb, NANB, NaNb, NANaNBNb, NNAB, R, Result;
    
    // timing
    chrono::high_resolution_clock::time_point time_start, time_end;
    
    // value N
    vector<uint64_t> N(slot_count, 9ULL);
    Plaintext plain_N;
    batch_encoder.encode(N, plain_N);
    
    // threshold
    vector<uint64_t> T(slot_count, 8ULL);
    Plaintext plain_T;
    batch_encoder.encode(T, plain_T);
    
    time_start = chrono::high_resolution_clock::now();
    
    // four additions
    evaluator.add(encrypted_A, encrypted_B, NA);
    evaluator.add(encrypted_C, encrypted_D, Na);
    evaluator.add(encrypted_A, encrypted_C, NB);
    evaluator.add(encrypted_B, encrypted_D, Nb);
    
    // three multiplications
    evaluator.multiply(NA, NB, NANB);
    evaluator.relinearize_inplace(NANB, relin_keys);
    
    cout << "Noise budget in NANB: "
    << decryptor.invariant_noise_budget(NANB) << " bits" << endl;
    
    evaluator.multiply(Na, Nb, NaNb);
    evaluator.relinearize_inplace(NaNb, relin_keys);
    
    cout << "Noise budget in NaNb: "
    << decryptor.invariant_noise_budget(NaNb) << " bits" << endl;
    
    evaluator.multiply(NANB, NaNb, NANaNBNb);
    evaluator.relinearize_inplace(NANaNBNb, relin_keys);
    
    cout << "Noise budget in NANaNBNb: "
    << decryptor.invariant_noise_budget(NANaNBNb) << " bits" << endl;
    
    // two scalar multiplications
    evaluator.multiply_plain(encrypted_A, plain_N, NNAB);
    evaluator.multiply_plain(NANaNBNb, plain_T, R);
    
    // one negation
    evaluator.negate_inplace(NANB);
    
    // one addition
    evaluator.add_inplace(NNAB, NANB);
    
    // one squar
    evaluator.square_inplace(NNAB);
    evaluator.relinearize_inplace(NNAB, relin_keys);
    
    cout << "Noise budget in NNAB: "
    << decryptor.invariant_noise_budget(NNAB) << " bits" << endl;
    
    // one negation
    evaluator.negate_inplace(R);
    
    // one addition
    evaluator.add(NNAB, R, Result);
    
    time_end = chrono::high_resolution_clock::now();
    auto time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);
    cout << "Done [" << time_diff.count() << " microseconds]" << endl;
}

