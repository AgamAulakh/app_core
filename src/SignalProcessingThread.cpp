#include "SignalProcessingThread.h"
#define RAW_SAMPLE_NUMBER 1024
LOG_MODULE_REGISTER(eegals_app_core_sig_proc, LOG_LEVEL_DBG);
K_THREAD_STACK_DEFINE(sig_proc_stack_area, SIG_PROC_THREAD_STACK_SIZE_B);
struct k_thread sig_proc_thread_data;

// k_work_q SignalProcessingThread::epoch_work_queue;
// K_WORK_DEFINE(process_ready_epoch, SignalProcessingThread::ProcessOneEpoch);

float32_t inputSignal[1024] = {0,0.713737136,1.200363228,1.318952679,1.067980105,0.585211078,0.094321587,-0.181315234,-0.104672597,0.32199295,0.957577273,1.575777154,1.951426415,1.946484067,1.562770674,0.941272763,0.30828146,-0.111024001,-0.178480184,0.104539382,0.598079446,1.077662348,1.320755618,1.192548841,0.69820424,-0.018425155,-0.729130152,-1.207950952,-1.316922175,-1.058155205,-0.572337885,-0.084236612,0.183933019,0.098105294,-0.335833425,-0.973871361,-1.588634503,-1.956134593,-1.94130867,-1.549617999,-0.924961462,-0.294701892,0.117158383,0.17542898,-0.114887082,-0.610939395,-1.08719904,-1.322329918,-1.18450896,-0.682534437,0.036846661,0.744380348,1.215310897,1.314665228,1.048190572,0.559463465,0.07428734,-0.186332478,-0.091323272,0.349799916,0.9901514,1.601339819,1.960607533,1.9359014,1.536322098,0.908646998,0.281257149,-0.123074671,-0.172162787,0.125361738,0.623787326,1.096587324,1.323674561,1.176244806,0.666730731,-0.055260871,-0.75948482,-1.222442,-1.312183016,-1.038089163,-0.546591412,-0.064476623,0.188512608,0.084327758,-0.363889424,-1.006413763,-1.61389023,-1.964844219,-1.930263482,-1.52288597,-0.892332998,-0.267950101,0.128771846,0.168682821,-0.135960371,-0.636619648,-1.105824374,-1.324788583,-1.167757654,-0.650796158,0.073664136,0.774440693,1.229343252,1.309476769,1.027853965,0.533725322,0.054807274,-0.190472455,-0.077120036,0.378098916,1.022654823,1.626282904,1.968843689,1.924396195,1.509312647,0.876023087,0.254783583,-0.134248944,-0.164990351,0.14667997,0.649432766,1.1149074,1.325671072,1.159048828,0.634733785,-0.092052815,-0.789245131,-1.236013697,-1.306547768,-1.017487997,-0.520868784,-0.045282076,0.19221112,0.069701439,-0.392425331,-1.038870959,-1.63851504,-1.972605037,-1.918300872,-1.49560519,-0.859720888,-0.241760396,0.139505052,0.161086698,-0.157517496,-0.662223094,-1.123833647,-1.326321169,-1.150119708,-0.618546707,0.110423266,0.803895331,1.242452432,1.303397346,1.006994308,0.508025387,0.035903773,-0.193727758,-0.062073352,0.406865577,1.055058552,1.650583873,1.976127407,1.911978895,1.481766691,0.843430019,0.228883304,-0.144539313,-0.156973234,0.168469878,0.674987046,1.132600396,1.326738072,1.140971722,0.60223805,-0.128771853,-0.818388528,-1.248658611,-1.300026887,-0.996375973,-0.495198713,-0.026675075,0.195021579,0.054237213,-0.421416536,-1.07121399,-1.662486677,-1.979410002,-1.905431697,-1.467800269,-0.827154093,-0.216155035,0.149350925,0.152651381,-0.17953402,-0.687721043,-1.141204965,-1.326921031,-1.13160635,-0.585810967,0.147094945,0.832721992,1.254631439,1.296437828,0.9856361,0.482392341,0.017598655,-0.196091844,-0.046194508,0.436075058,1.087333662,1.67422076,1.982452075,1.898660764,1.453709071,0.81089672,0.203578281,-0.153939137,-0.148122613,0.190706795,0.700421511,1.149644708,1.326869351,1.122025124,0.569268641,-0.165388917,-0.846893031,-1.260370177,-1.292631653,-0.974777821,-0.469609841,-0.008677147,0.196937873,0.037946775,-0.45083797,-1.103413969,-1.685783471,-1.985252935,-1.891667631,-1.439496273,-0.794661499,-0.191155695,0.158303255,0.143388456,-0.20198505,-0.713084884,-1.157917018,-1.326582392,-1.112229625,-0.55261428,0.183650151,0.860898992,1.265874141,1.288609901,0.963804296,0.456854779,-8.69E-05,-0.197559037,-0.029495603,0.465702067,1.119451314,1.697172193,1.987811948,1.884453885,1.425165076,0.778452028,0.178889893,-0.16244264,-0.138450481,0.213365607,0.725707602,1.166019327,1.326059567,1.102221484,0.535851118,-0.201875036,-0.874737259,-1.271142701,-1.284374157,-0.952718711,-0.444130713,0.008690782,0.197954762,0.02084263,-0.480664124,-1.135442112,-1.708384352,-1.990128532,-1.877021161,-1.410718708,-0.76227189,-0.166783451,0.166356705,0.133310315,-0.224845261,-0.738286115,-1.173949105,-1.325300346,-1.092002383,-0.518982416,0.22005997,0.888405256,1.276175282,1.279926058,0.941524278,0.43144119,-0.017132125,-0.198124531,-0.011989542,0.495720887,1.151382786,1.719417411,1.992202161,1.869371144,1.396160422,0.746124665,0.154838905,-0.170044921,-0.127969629,0.236420783,0.75081688,1.181703862,1.324304253,1.081574051,0.502011459,-0.238201361,-0.901900445,-1.280971364,-1.27526729,-0.930224232,-0.418789749,0.025408402,0.19806788,0.002938076,-0.510869079,-1.167269767,-1.730268874,-1.994032365,-1.861505569,-1.381493495,-0.730013919,-0.143058754,0.173506811,0.122430145,-0.248088919,-0.763296367,-1.189281148,-1.323070868,-1.070938269,-0.484941554,0.256295626,0.915220333,1.285530483,1.270399585,0.918821833,0.40617992,-0.033517172,-0.1977844,0.006309982,0.526105401,1.183099498,1.740936286,1.995618727,1.853426219,1.366721226,0.713943211,0.131445452,-0.176741956,-0.116693635,0.259846393,0.775721053,1.196678554,1.321599825,1.060096864,0.467776035,-0.274339192,-0.928362462,-1.289852229,-1.265324728,-0.907320365,-0.39361522,0.04145604,0.197273739,-0.0157528,-0.541426527,-1.198868434,-1.751417232,-1.996960888,-1.845134926,-1.35184694,-0.697916085,-0.120001416,0.17974999,0.110761916,-0.271689905,-0.78808743,-1.203893714,-1.319890814,-1.049051713,-0.450518255,0.292328502,0.941324421,1.293936248,1.260044548,0.895723131,0.381099154,-0.049222647,-0.196535598,0.025388497,0.556829114,1.214573041,1.761709341,1.998058543,1.836633567,1.336873981,0.681936076,0.108729019,-0.182530604,-0.104636855,0.283616136,0.800392003,1.210924302,1.31794358,1.037804737,0.43317159,-0.310260007,-0.954103837,-1.297782241,-1.254560924,-0.884033459,-0.368635215,0.05681468,0.195569736,-0.035215145,-0.572309795,-1.230209799,-1.771810283,-1.998911442,-1.827924071,-1.321805715,-0.666006703,-0.097630591,0.185083543,0.098320365,-0.295621742,-0.812631286,-1.217768035,-1.315757924,-1.026357908,-0.415739438,0.328130174,0.966698383,1.301389965,1.248875782,0.872254696,0.356226883,-0.064229868,-0.194375964,0.045230767,0.587865182,1.245775201,1.78171777,1.999519393,1.81900841,1.30664553,0.650131472,0.08670842,-0.187408609,-0.091814406,0.307703364,0.824801813,1.224422673,1.313333703,1.014713243,0.398225213,-0.345935484,-0.979105771,-1.304759233,-1.242991092,-0.860390209,-0.343877623,0.071465982,0.192954151,-0.055433344,-0.603491871,-1.261265756,-1.791429559,-1.999882257,-1.809888605,-1.291396832,-0.634313877,-0.075964751,0.189505657,0.085120983,-0.319857619,-0.836900129,-1.230886021,-1.310670828,-1.002872806,-0.380632353,0.363672434,0.991323762,1.307889912,1.236908872,0.848443384,0.331590883,-0.078520837,-0.191304221,0.065820807,0.619186434,1.276677988,1.800943451,1.999999951,1.800566722,1.276063046,0.618557392,0.065401785,-0.191374599,-0.07824215,0.332081109,0.848922796,1.237155928,1.307769267,0.990838706,0.362964312,-0.381337534,-1.003350156,-1.310781926,-1.230631187,-0.836417626,-0.319370097,0.085392294,0.189426154,-0.076391043,-0.63494543,-1.292008437,-1.810257291,-1.999872449,-1.791044873,-1.260647615,-0.602865477,-0.055021677,0.193015403,0.071180003,-0.344370416,-0.860866392,-1.243230285,-1.304629042,-0.978613097,-0.345224561,0.398927315,1.015182803,1.313435254,1.224160146,0.824316355,0.307218682,-0.092078257,-0.187319983,0.087141894,0.650765398,1.30725366,1.81936897,1.99949978,1.781325213,1.245153999,0.587241575,0.044826539,-0.194428092,-0.063936684,0.356722105,0.872727515,1.249107032,1.301250234,0.966198179,0.327416589,-0.416438322,-1.026819593,-1.31584993,-1.217497901,-0.812143012,-0.295140036,0.098576676,0.1849858,-0.098071159,-0.666642861,-1.322410233,-1.828276422,-1.998882028,-1.771409944,-1.229585676,-0.571689111,-0.034818437,0.195612745,0.056514379,-0.369132728,-0.884502779,-1.254784152,-1.297632975,-0.953596195,-0.309543901,0.433867121,1.038258466,1.318026046,1.210646651,0.79990105,0.28313754,-0.104885546,-0.18242375,0.10917659,0.682574327,1.337474751,1.836977631,1.998019335,1.761301312,1.213946136,0.556211489,0.02499939,-0.196569495,-0.048915319,0.381598819,0.896188817,1.260259674,1.293777455,0.940809432,0.291610016,-0.451210295,-1.049497407,-1.319963747,-1.203608636,-0.787593939,-0.271214554,0.11100291,0.179634033,-0.120455898,-0.698556289,-1.352443828,-1.845470623,-1.996911894,-1.751001605,-1.198238888,-0.540812096,-0.015371372,0.197298531,0.041141777,-0.394116897,-0.907782284,-1.265531677,-1.289683921,-0.927840221,-0.273618468,0.468464449,1.060534445,1.321663234,1.196386142,0.775225162,0.25937442,-0.116926856,-0.176616908,0.131906751,0.714585226,1.367314098,1.853753475,1.995559959,1.740513154,1.182467453,0.525494297,0.005936308,-0.197800099,-0.033196068,0.40668347,0.919279853,1.270598283,1.285352673,0.914690934,0.255572805,-0.485626208,-1.071367661,-1.323124764,-1.188981494,-0.762798216,-0.247620456,0.122655519,0.173372686,-0.143526773,-0.730657604,-1.382082216,-1.861824309,-1.993963836,-1.729838333,-1.166635363,-0.510261438,0.003303921,0.198074498,0.025080551,-0.419295031,-0.930678221,-1.275457664,-1.280784068,-0.901363987,-0.237476587,0.502692219,1.08199518,1.32434865,1.181397063,0.750316611,0.235955962,-0.128187085,-0.169901734,0.155313547,0.746769877,1.396744862,1.869681296,1.992123887,1.718979559,1.150746166,0.495116839,-0.012347486,-0.198122085,-0.016797624,0.431948063,0.941974104,1.280108038,1.275978516,0.887861837,0.219333385,-0.519659149,-1.092415176,-1.32533526,-1.173635258,-0.737783868,-0.224384213,0.133519783,0.166204475,-0.167264617,-0.762918487,-1.411298734,-1.877322654,-1.99004053,-1.707939288,-1.134803419,-0.480063802,0.021192602,0.197943268,0.00834973,-0.444639036,-0.953164246,-1.284547675,-1.270936486,-0.874186981,-0.201146784,0.536523693,1.102625871,1.326085015,1.165698531,0.725203519,0.212908461,-0.138651893,-0.162281387,0.179377482,0.779099867,1.425740557,1.884746651,1.987714239,1.696720017,1.11881069,0.465105601,-0.029837536,-0.197538515,0.000260652,0.45736441,0.96424541,1.28877489,1.265658498,0.860341956,0.182920375,-0.553282565,-1.112625538,-1.326598395,-1.157589373,-0.712579106,-0.201531935,0.143581746,0.158133001,-0.191649606,-0.795310438,-1.440067079,-1.891951603,-1.98514554,-1.685324286,-1.102771557,-0.450245489,0.038280601,0.196908345,-0.009031,-0.470120638,-0.975214386,-1.29278805,-1.260145131,-0.846329342,-0.164657763,0.569932507,1.122412498,1.326875932,1.149310316,0.699914181,0.190257838,-0.148307718,-0.153759907,0.204078409,0.811546613,1.454275075,1.898935878,1.982335019,1.67375467,1.086689607,0.435486692,-0.046520163,-0.196053335,0.017958755,0.482904161,0.98606799,1.296585569,1.254397015,0.832151754,0.146362556,-0.586470285,-1.13198512,-1.326918213,-1.140863929,-0.687212303,-0.179089348,0.152828238,0.149162745,-0.216661275,-0.827804798,-1.468361342,-1.905697891,-1.979283311,-1.662013786,-1.070568435,-0.420832411,0.054554635,0.194974113,-0.027041317,-0.495711414,-0.996803061,-1.300165912,-1.248414839,-0.817811849,-0.128038374,0.602892691,1.141341825,1.326725883,1.132252822,0.674477039,0.168029617,-0.157141783,-0.144342212,0.229395548,0.844081391,1.482322708,1.912236108,1.975991111,1.650104289,1.054411642,0.406285821,-0.06238248,-0.193671366,0.036276051,0.508538826,1.007416469,1.303527597,1.242199342,0.803312319,0.109688843,-0.619196545,-1.150481085,-1.326299637,-1.123479641,-0.661711963,-0.157081769,0.161246882,0.139299061,-0.242278537,-0.860372782,-1.496156025,-1.918549046,-1.972459166,-1.638028871,-1.038222838,-0.391850068,0.070002214,0.192145831,-0.045660282,-0.521382818,-1.017905109,-1.306669188,-1.235751322,-0.788655896,-0.091317593,0.635378695,1.15940142,1.325640228,1.114547071,0.648920655,0.146248902,-0.165142114,-0.134034097,0.25530751,0.87667536,1.509858176,1.924635274,1.968688276,1.625790261,1.022005635,0.377528271,-0.077412403,-0.190398302,0.055191299,0.534239806,1.028265904,1.309589305,1.229071627,0.773845347,0.072928262,-0.651436016,-1.168101405,-1.324748462,-1.105457831,-0.6361067,-0.135534083,0.168826108,0.128548178,-0.268479703,-0.892985503,-1.523426069,-1.930493411,-1.964679299,-1.613391224,-1.005763653,-0.363323519,0.084611662,0.188429626,-0.064866356,-0.547106204,-1.038495808,-1.312286615,-1.222161163,-0.758883475,-0.054524488,0.667365412,1.176579663,1.323625197,1.096214678,0.623273685,0.124940351,-0.172297547,-0.12284222,0.281792314,0.909299592,1.536856646,1.936122127,1.960433145,1.600834561,0.989500514,0.349238875,-0.09159866,-0.186240705,0.07468267,0.55997842,1.048591803,1.314759839,1.215020886,0.743773119,0.036109917,-0.68316382,-1.18483487,-1.322271348,-1.086820403,-0.610425203,-0.114470717,0.175555164,0.116917189,-0.295242505,-0.925613999,-1.550146876,-1.941520146,-1.955950776,-1.588123109,-0.973219843,-0.335277366,0.098372117,0.183832492,-0.084637424,-0.57285286,-1.058550904,-1.31700775,-1.20765181,-0.728517154,-0.017688195,0.698828204,1.192865754,1.320687883,1.077277834,0.597564849,0.104128157,-0.178597744,-0.110774105,0.308827406,0.941925098,1.56329376,1.946686243,1.95123321,1.575259739,0.956925268,0.321441994,-0.104930806,-0.181205996,0.094727768,0.585725929,1.068370154,1.319029173,1.200054997,0.713118486};

SignalProcessingThread::SignalProcessingThread() {
    epoch_count = 0;
    // NEED TO DO CHECKS
};

void SignalProcessingThread::Initialize() {
    LOG_DBG("SigProc::%s -- initializing Sig Processing", __FUNCTION__);

    if (id == nullptr) {
        LOG_DBG("SigProc::%s -- making thread", __FUNCTION__);
        id = k_thread_create(
            &sig_proc_thread_data, sig_proc_stack_area,
            K_THREAD_STACK_SIZEOF(sig_proc_stack_area),
            SignalProcessingThread::RunThreadSequence,
            this, NULL, NULL,
            SIG_PROC_THREAD_PRIORITY, 0, K_NO_WAIT
        );

        k_thread_name_set(id, "SignalProcessingThread");
        LOG_DBG("SigProc::%s -- thread create successful", __FUNCTION__);
    }
};

void SignalProcessingThread::Run() {
    uint8_t message = 0;
    while (true) {
        if (message_queue.get_with_blocking_wait(message)) {
            uint8_t message_enum = static_cast<SignalProcessingThreadMessage>(message);
		    LOG_DBG("SigProc::%s -- received message: %u at: %u ms", __FUNCTION__, message_enum, k_uptime_get_32());
            switch (message_enum) {
                case COMPUTE_DEBUG_FFT_RESULTS:
					TestValuesWooHoo();
                   // ComputeSingleSideFFT();
                    break;
                case COMPUTE_DEBUG_POWER_RESULTS:
					TestValuesWooHoo();
                    ComputeSingleSidePower();
                    break;
                case COMPUTE_DEBUG_BANDPOWER_RESULTS:
					TestValuesWooHoo();
                    ComputeSingleSidePower();
                    //ComputeBandPowers(PowerBands::DELTA);
                    break;
                case COMPUTE_DEBUG_RELATIVEPOWER_RESULTS:
                    ComputeRelativeBandPowers();
                    break;
                case START_PROCESSING:
                    epoch_count = 0;
                    StartProcessing();
                    // to be set by state machine
                    break;
                case INVALID:
                    break;
                default:
                    break;
            }
        }
    }
};

void SignalProcessingThread::StartProcessing()
{
    // wait until you can read one full epoch
    while(epoch_count < max_epochs){
        if(DataBufferManager::epoch_lock.wait()) {
            DataBufferManager::ReadEpoch(allChannels);
            DataBufferManager::epoch_lock.give();
            DataBufferManager::DoneReadingEpoch();
            ComputeSingleSidePower();
            ComputeBandPowers();
            epoch_count++;
        }
    }
    k_event_post(&s_obj.sig_proc_complete, EVENT_SIG_PROC_COMPLETE);
    // stop sigproc
}

void SignalProcessingThread::ProcessOneEpoch()
{
    // DataBufferManager::ReadEpoch(allChannels);
    // ComputeSingleSidePower();
    // ComputeBandPowers();

    // if(++epoch_count >= max_epochs) {
    //     // tell state machine we done
    // }
}

void SignalProcessingThread::TestValuesWooHoo()
{
    printk("\nFilling up allChannels with sample data Woo Hoo\n");
	for (int i = 0; i < RAW_SAMPLE_NUMBER; i++) {
       allChannels.set_at(inputSignal[i], i, 0);
    }
    allChannels.rawFFT(0);
    //allChannels.prettyPrint();

}

void SignalProcessingThread::ComputeSingleSideFFT()
{	
    printk("\nPrint single-sided FFT results:\n");
    for (int i = 0; i < num_electrodes; i++) {
        channelFFTResults.set_column_vector_at(allChannels.singleSideFFT(i), i);
    }

}
 

void SignalProcessingThread::ComputeSingleSidePower()
{
    printk("\nPrint single-sided power results:\n");
    for (int i = 0; i < num_electrodes; i++) 
    {
        channelPowerResults.set_column_vector_at(allChannels.singleSidePower(i), i);
    }
    
}

void SignalProcessingThread::ComputeBandPowerAtOneBand(const PowerBands powerBand)
{
	printk("\nPrint band power results:\n");
    uint8_t powerBand_enum = static_cast<PowerBands>(powerBand);
    // Computation of bandpowers
    switch (powerBand_enum) {
        case DELTA:{
            for (int i = 0; i < num_electrodes; i++) {   
                channelBandPowers.set_at(
                    channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, DELTA), DELTA, i
                );
				printk("\nPrint delta power results: %.4f\n", channelBandPowers.at(DELTA,i));
			}
            break;
        }   
        case THETA:{
            for (int i = 0; i < num_electrodes; i++) {   
                channelBandPowers.set_at(
                    channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, THETA), THETA, i
                );
				printk("\nPrint theta power results: %.4f\n", channelBandPowers.at(THETA,i));
            }
            break;
        }
        case ALPHA:{
            for (int i = 0; i < num_electrodes; i++) { 
                channelBandPowers.set_at(
                    channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, ALPHA), ALPHA, i
                );  
				printk("\nPrint alpha power results: %.4f\n", channelBandPowers.at(ALPHA,i));
		    }
            break;
        }
        case BETA:{
            for (int i = 0; i < num_electrodes; i++) {
                channelBandPowers.set_at(
                    channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, BETA), BETA, i
                );
				printk("\nPrint beta power results: %.4f\n", channelBandPowers.at(BETA,i));
			}
            break;
        }
        case INVALID:
            break;
        default:
            break;
    }
}

void SignalProcessingThread::ComputeBandPowers() {
	printk("\nPrint band power results:\n");

    for (int i = 0; i < num_electrodes; i++) {   
        channelBandPowers.set_at(
            channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, DELTA), DELTA, i
        );
        channelBandPowers.set_at(
            channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, THETA), THETA, i
        );
        channelBandPowers.set_at(
            channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, ALPHA), ALPHA, i
        );
        channelBandPowers.set_at(
            channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, BETA), BETA, i
        );
    }

    channelBandPowers.prettyPrint();
}

void SignalProcessingThread::ComputeRelativeBandPowers()
{
     for (int i = 0; i < num_electrodes; i++) 
    {
        // Each channel's power spectrum calculates the relative band powers of the 4 bands and stores them into 
        // a 2D array. Outer index denotes the channel number, inner index denotes the band power value
        // channelRelativeBandPowers[i] = channelPowerResults[i].singleSideRelativeBandPower(channelBandPowers[i]);
    }
}


