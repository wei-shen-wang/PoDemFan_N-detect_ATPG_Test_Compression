# ATPG_TDF_ndetect_compression
2023 VLSI Testing Final Project

In all three directories FAN_PODEM_PARALLEL, FANV2_PODEMV1, PODEM
- type make to compile the code into executables
- type make clean to clean the executables

Command :
```sh
./atpg -tdfatpg -ndet n -compression *.ckt > *.pat
```
## PODEM

Implement and modify the PODEM algorothim to generate patterns for transition delay fault.
This part is done by [Hsin-Tzu Zhang](https://github.com/EE08053).

## PODEM V1 + FAN V2

Implement FAN to generate V2 and PODEM to generate V1 for transition delay faults. 
This part is done by [Wei-Shen Wang](https://github.com/wei-shen-wang).

The circuit ckt-to-verilog conversion for FAN_ATPG is done by [Hsin-Tzu Zhang](https://github.com/EE08053) and [Yu-Hung Pan](https://github.com/PAN-YU-HUNG).

The modified command line is done by [Yu-Hung Pan](https://github.com/PAN-YU-HUNG).

In this part, we implement FAN ATPG utilizing the open source code FAN_ATPG. The original open source code only generate pattern set for stuck-at faults. Therefore, we need to implement transition-delay-fault LOS ATPG by adding some functions to the open source code. Following are some important codes and functions.

There are two flows we implemented. 

The first and default flow is v1 first test pattern generation. It involves using the PODEM algoritm to generate V1 first, and then we shift the pattern and do logic simulation. With some gates' values specified, we use the values as a constraint and use the FAN algorothim to generate V2.

The second and backup flow is v2 first test pattern generation. It involves using the FAN algoritm to generate V2 first, and then we shift back the pattern and do logic simulation. With some gates' values specified, we use the values as a constraint and use the PODEM algorothim to generate V1.

As for the modified details, please scroll to the bottom of this readme or take a direct look at the source codes.

Original FAN algorithm source code : [FAN ATPG](https://github.com/NTU-LaDS-II/FAN_ATPG).

<!---
This part is deprecated

### Additional command (debug use)

To run the fault simulation for transition delay fault in FAN package, you can type the command :
```sh
./atpg -ndet n -tdfsim *.pat *.ckt
```

To generate the pattern with specific pattern format for FAN, you can additionally type **-rpf** flag. This will additionally generate the pattern file **./tdf_test.pat**.
--->

## Parallel Processing

Run two ATPG in parallel and choose better generated pattern. This part is done by [Yu-Hung Pan](https://github.com/PAN-YU-HUNG).

## Statistics for reference

Ndet : **8**, Compression : **ON**

PODEM :
| Circuit | FC(%) | TL | RT(s) |
| :----: | :----: | :----: | :----: |
|c432|11.62% |170 |0.8|
|c499|94.69%|582|16.6|
|c880|50.38%|299|8.7|
|c1355|38.41%|463|5.0|
|c2670|94.06%|874|60.3|
|c3540|23.26%|486|76.8|
|c6288|97.63%|387|75.2|
|c7522|98.31%|1199|433.5|
|Average|63.55%|557.5 |84.6|

PODEM V1 + FAN V2 :
| Circuit | FC(%) | TL | RT(s) |
| :----: | :----: | :----: | :----: |
|c432|11.62%|170|0.51|
|c499|95.56%|394|7.28|
|c880|50.33%|368|22.28|
|c1355|38.41%|469|7.45|
|c2670|94.13%|883|428|
|c3540|23.26%|495|190.4|
|c6288|97.66%|375|322.7|
|c7522|98.21%|1945|1176|
|Average|63.65%|637.4|269.3|

## Appendix
As shown in the following code, we try V1 first pattern generation as default(`generateTDFV1_by_PODEM_first`). If the default flow fail, we try V2 first pattern generation(`generateSinglePatternOnTargetTDF`).

```c++
SINGLE_PATTERN_GENERATION_STATUS result;
result = generateTDFV1_by_PODEM_first(fTDF, pattern);
if (result != PATTERN_FOUND)
{
	for (int i = 0; i < pattern.PI1_.size(); ++i)
	{
		pattern.PI1_[i] = X;
		pattern.PI2_[i] = X;
	}
	result = generateSinglePatternOnTargetTDF(fTDF, pattern, false);
}

```
The `generateTDFV1_by_PODEM_first` function is implemented as follow.
```c++
Atpg::SINGLE_PATTERN_GENERATION_STATUS Atpg::generateTDFV1_by_PODEM_first(Fault targetFault, Pattern &pattern)
{
	int numOfBacktrack = 0; // backtrack times
	SINGLE_PATTERN_GENERATION_STATUS genStatus = PATTERN_FOUND;
	std::vector<bool> gateID2changed(this->pCircuit_->numGate_, false);
	std::vector<bool> gateID2scheduled(this->pCircuit_->numGate_, false);
	std::vector<bool> gateID2backtracked(this->pCircuit_->numGate_, false);
	int faulty_GateID = (targetFault.faultyLine_ == 0) ? targetFault.gateID_ : pCircuit_->circuitGates_[targetFault.gateID_].faninVector_[targetFault.faultyLine_ - 1];
	Gate &atpgForV1_faulty_gate = this->pCircuit_->circuitGates_[faulty_GateID];
	Value faultActivationValue;
	std::stack<int> decisionTree_for_V1;
	if (targetFault.faultType_ == Fault::STR)
	{
		faultActivationValue = L;
	}
	else if (targetFault.faultType_ == Fault::STF)
	{
		faultActivationValue = H;
	}
	// assign PI to atpg
	for (int i = 0; i < this->pCircuit_->numGate_; ++i)
	{
		if (i < this->pCircuit_->numPI_)
		{
			switch (pattern.PI1_[i])
			{
				case X:
					if (this->pCircuit_->circuitGates_[i].atpgVal_ != X)
					{
						this->pCircuit_->circuitGates_[i].atpgVal_ = X;
						gateID2changed[i] = true;
					}
					break;
				case H:
					if (faulty_GateID == i)
					{
						if (faultActivationValue != H)
						{
							return FAULT_UNTESTABLE;
						}
					}
					if (this->pCircuit_->circuitGates_[i].atpgVal_ != H)
					{
						this->pCircuit_->circuitGates_[i].atpgVal_ = H;
						gateID2changed[i] = true;
					}
					break;
				case L:
					if (faulty_GateID == i)
					{
						if (faultActivationValue != L)
						{
							return FAULT_UNTESTABLE;
						}
					}
					if (this->pCircuit_->circuitGates_[i].atpgVal_ != L)
					{
						this->pCircuit_->circuitGates_[i].atpgVal_ = L;
						gateID2changed[i] = true;
					}
					break;
			}
		}
		else
		{
			this->pCircuit_->circuitGates_[i].atpgVal_ = X;
		}
	}

	do
	{
		for (int i = 0; i < this->pCircuit_->numPI_; ++i)
		{
			if (gateID2changed[i])
			{
				gateID2changed[i] = false;
				for (int j = 0; j < this->pCircuit_->circuitGates_[i].numFO_; ++j)
				{
					gateID2scheduled[this->pCircuit_->circuitGates_[i].fanoutVector_[j]] = true;
				}
			}
		}

		for (int i = this->pCircuit_->numPI_; i < this->pCircuit_->numGate_; ++i)
		{
			if (gateID2scheduled[i])
			{
				gateID2scheduled[i] = false;
				Value originalVal = this->pCircuit_->circuitGates_[i].atpgVal_;
				this->pCircuit_->circuitGates_[i].atpgVal_ = this->evaluateGoodVal(this->pCircuit_->circuitGates_[i]);
				if (originalVal != this->pCircuit_->circuitGates_[i].atpgVal_)
				{
					for (int j = 0; j < this->pCircuit_->circuitGates_[i].numFO_; ++j)
					{
						gateID2scheduled[this->pCircuit_->circuitGates_[i].fanoutVector_[j]] = true;
					}
				}
			}
		}

		// if fault activated, v1 found
		if (faultActivationValue == atpgForV1_faulty_gate.atpgVal_)
		{
			// gen pattern 2 here
			genStatus = this->generateTDFV2_by_FAN_second(targetFault, pattern, false);
			if (genStatus == TDF_V2_FAIL)
			{
				goto PODEM_BACKTRACKING;
			}
			else if (genStatus == TDF_V2_FOUND)
			{
				genStatus = PATTERN_FOUND;
			}
			else
			{
				std::cerr << "unexpected result in generateTDFV1_by_PODEM_first\n";
			}
			break;
		}
		else if (atpgForV1_faulty_gate.atpgVal_ == X)
		{
			// find pi assignment
			Value piValueToAssign = faultActivationValue;
			Gate *pObjectGate = &atpgForV1_faulty_gate;
			Gate *pNextObjectGate = NULL;

			while (pObjectGate->gateId_ >= pCircuit_->numPI_)
			{
				// std::cerr << "gate id " << pObjectGate->gateId_ << "\n";
				// std::cerr << "gate fanin id " << reinitializedCircuit.circuitGates_[pObjectGate->faninVector_[0]].gateId_ << "\n";
				// exit(0);
				pNextObjectGate = NULL;
				// choose object gate index
				switch (pObjectGate->gateType_)
				{
					case Gate::OR2:
					case Gate::OR3:
					case Gate::OR4:
					case Gate::OR5:
						if (piValueToAssign == H)
						{
							for (int j = 0; j < pObjectGate->numFI_; ++j)
							{
								if (this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]].atpgVal_ == X)
								{
									if (!pNextObjectGate)
									{
										pNextObjectGate = &(this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]]);
									}
									else
									{
										if (pNextObjectGate->cc1_ < this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]].cc1_)
										{
											pNextObjectGate = &(this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]]);
										}
									}
								}
							}
						}
						else if (piValueToAssign == L)
						{
							for (int j = 0; j < pObjectGate->numFI_; ++j)
							{
								if (this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]].atpgVal_ == X)
								{
									if (!pNextObjectGate)
									{
										pNextObjectGate = &(this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]]);
									}
									else
									{
										if (pNextObjectGate->cc0_ > this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]].cc0_)
										{
											pNextObjectGate = &(this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]]);
										}
									}
								}
							}
						}
						break;
					case Gate::NAND2:
					case Gate::NAND3:
					case Gate::NAND4:
						if (piValueToAssign == H)
						{
							for (int j = 0; j < pObjectGate->numFI_; ++j)
							{
								if (this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]].atpgVal_ == X)
								{
									if (!pNextObjectGate)
									{
										pNextObjectGate = &(this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]]);
									}
									else
									{
										if (pNextObjectGate->cc0_ < this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]].cc0_)
										{
											pNextObjectGate = &(this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]]);
										}
									}
								}
							}
						}
						else if (piValueToAssign == L)
						{
							for (int j = 0; j < pObjectGate->numFI_; ++j)
							{
								if (this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]].atpgVal_ == X)
								{
									if (!pNextObjectGate)
									{
										pNextObjectGate = &(this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]]);
									}
									else
									{
										if (pNextObjectGate->numLevel_ > this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]].numLevel_)
										{
											pNextObjectGate = &(this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]]);
										}
									}
								}
							}
						}
						break;
					case Gate::NOR2:
					case Gate::NOR3:
					case Gate::NOR4:
						if (piValueToAssign == H)
						{
							for (int j = 0; j < pObjectGate->numFI_; ++j)
							{
								if (this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]].atpgVal_ == X)
								{
									if (!pNextObjectGate)
									{
										pNextObjectGate = &(this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]]);
									}
									else
									{
										if (pNextObjectGate->cc0_ > this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]].cc0_)
										{
											pNextObjectGate = &(this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]]);
										}
									}
								}
							}
						}
						else if (piValueToAssign == L)
						{
							for (int j = 0; j < pObjectGate->numFI_; ++j)
							{
								if (this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]].atpgVal_ == X)
								{
									if (!pNextObjectGate)
									{
										pNextObjectGate = &(this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]]);
									}
									else
									{
										if (pNextObjectGate->cc1_ < this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]].cc1_)
										{
											pNextObjectGate = &(this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]]);
										}
									}
								}
							}
						}
						break;
					case Gate::AND2:
					case Gate::AND3:
					case Gate::AND4:
					case Gate::AND5:
					case Gate::AND8:
					case Gate::AND9:
						if (piValueToAssign == H)
						{
							for (int j = 0; j < pObjectGate->numFI_; ++j)
							{
								if (this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]].atpgVal_ == X)
								{
									if (!pNextObjectGate)
									{
										pNextObjectGate = &(this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]]);
									}
									else
									{
										if (pNextObjectGate->cc1_ > this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]].cc1_)
										{
											pNextObjectGate = &(this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]]);
										}
									}
								}
							}
						}
						else if (piValueToAssign == L)
						{
							for (int j = 0; j < pObjectGate->numFI_; ++j)
							{
								if (this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]].atpgVal_ == X)
								{
									if (!pNextObjectGate)
									{
										pNextObjectGate = &(this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]]);
									}
									else
									{
										if (pNextObjectGate->cc0_ < this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]].cc0_)
										{
											pNextObjectGate = &(this->pCircuit_->circuitGates_[pObjectGate->faninVector_[j]]);
										}
									}
								}
							}
						}
						break;
					case Gate::INV:
					case Gate::BUF:
						if (this->pCircuit_->circuitGates_[pObjectGate->faninVector_[0]].atpgVal_ == X)
						{
							pNextObjectGate = &(this->pCircuit_->circuitGates_[pObjectGate->faninVector_[0]]);
						}
						break;
				}

				switch (pObjectGate->gateType_)
				{
					case Gate::INV:
					case Gate::NOR2:
					case Gate::NOR3:
					case Gate::NOR4:
					case Gate::NAND2:
					case Gate::NAND3:
					case Gate::NAND4:
						if (piValueToAssign == H)
						{
							piValueToAssign = L;
						}
						else if (piValueToAssign == L)
						{
							piValueToAssign = H;
						}
						break;
				}
				pObjectGate = pNextObjectGate;
				if (pNextObjectGate == NULL)
				{
					genStatus = FAULT_UNTESTABLE;
					break;
				}
				if (pObjectGate == NULL)
				{
					break;
				}
			}
			if (pObjectGate == NULL)
			{
				genStatus = FAULT_UNTESTABLE;
				break;
			}
			else
			{
				assert(pObjectGate->gateId_ < this->pCircuit_->numPI_);
				assert(piValueToAssign != X);
				pObjectGate->atpgVal_ = piValueToAssign;
				gateID2changed[pObjectGate->gateId_] = true;
				decisionTree_for_V1.push(pObjectGate->gateId_);
			}
		}
		// else backtrack assigned PI
		else
		{
		PODEM_BACKTRACKING:
			while (true)
			{
				if (numOfBacktrack > PODEM_LIMIT)
				{
					genStatus = ABORT;
					break;
				}
				if (decisionTree_for_V1.empty())
				{
					genStatus = FAULT_UNTESTABLE;
					break;
				}
				int backtrackCandidate = decisionTree_for_V1.top();
				if (gateID2backtracked[backtrackCandidate] == true)
				{
					gateID2backtracked[backtrackCandidate] = false;
					this->pCircuit_->circuitGates_[backtrackCandidate].atpgVal_ = X;
					gateID2changed[backtrackCandidate] = true;
					decisionTree_for_V1.pop();
				}
				else
				{
					if (this->pCircuit_->circuitGates_[backtrackCandidate].atpgVal_ == H)
					{
						this->pCircuit_->circuitGates_[backtrackCandidate].atpgVal_ = L;
					}
					else if (this->pCircuit_->circuitGates_[backtrackCandidate].atpgVal_ == L)
					{
						this->pCircuit_->circuitGates_[backtrackCandidate].atpgVal_ = H;
					}
					gateID2changed[backtrackCandidate] = true;
					gateID2backtracked[backtrackCandidate] = true;
					++numOfBacktrack;
					break;
				}
			}
		}
		if (genStatus == FAULT_UNTESTABLE || genStatus == ABORT)
		{
			break;
		}
	} while (true);

	// both v1 and v2 should be found and has no contradiction
	if (genStatus == PATTERN_FOUND)
	{

		pattern.PI1_[this->pCircuit_->numPI_] = this->pCircuit_->circuitGates_[this->pCircuit_->numPI_].atpgVal_;
		for (int i = 0; i < this->pCircuit_->numPI_ - 1; ++i)
		{
			pattern.PI1_[i] = pattern.PI2_[i + 1];
		}
	}
	return genStatus;
}
```

Notice that the `generateTDFV2_by_FAN_second` function is called in `generateTDFV1_by_PODEM_first`
Basically we just make a copy of ATPG of the current ATPG and try to generated V2 with the FAN algorithm with it. The function is implemented in code as follow.

```c++
Atpg::SINGLE_PATTERN_GENERATION_STATUS Atpg::generateTDFV2_by_FAN_second(Fault targetFault, Pattern &pattern, bool isAtStageDTC)
{
	static int functionCall = 0;
	++functionCall;
	// std::cerr << functionCall << " generateTDFV2_by_FAN_second start\n";
	Circuit reinitializedCircuit = *(this->pCircuit_);
	Simulator reinitializedSimulator = Simulator(&reinitializedCircuit);
	Atpg reinitializedAtpg = Atpg(&reinitializedCircuit, &reinitializedSimulator);
	int backwardImplicationLevel = 0;													
	int numOfBacktrack = 0;																
	bool Finish = false;																
	bool faultHasPropagatedToPO = false;											 
	Gate *pFaultyLine = NULL;														
	Gate *pLastDFrontier = NULL;													
	IMPLICATION_STATUS implicationStatus;											
	BACKTRACE_STATUS backtraceFlag;													
	SINGLE_PATTERN_GENERATION_STATUS genStatus = TDF_V2_FOUND;
	std::vector<bool> gateID2changed(reinitializedCircuit.numGate_, false);
	std::vector<bool> gateID2scheduled(reinitializedCircuit.numGate_, false);
	// try to use copy instead of setting up again
	reinitializedAtpg.gateID_to_lineType_ = this->gateID_to_lineType_;
	reinitializedAtpg.numOfheadLines_ = this->numOfheadLines_;
	reinitializedAtpg.headLineGateIDs_ = this->headLineGateIDs_;
	reinitializedAtpg.gateID_to_uniquePath_ = this->gateID_to_uniquePath_;

	for (int i = 0; i < reinitializedCircuit.numGate_; ++i)
	{
		reinitializedCircuit.circuitGates_[i].depthFromPo_ = this->pCircuit_->circuitGates_[i].depthFromPo_;
		if (0 < i && i < reinitializedCircuit.numPI_)
		{
			reinitializedCircuit.circuitGates_[i].atpgVal_ = this->pCircuit_->circuitGates_[i - 1].atpgVal_;
			if (reinitializedCircuit.circuitGates_[i].atpgVal_ != X)
			{
				gateID2changed[i] = true;
			}
		}
		else
		{
			reinitializedCircuit.circuitGates_[i].atpgVal_ = X;
		}
	}

	for (int i = 0; i < reinitializedCircuit.numPI_; ++i)
	{
		if (gateID2changed[i])
		{
			gateID2changed[i] = false;
			Value originalVal = reinitializedCircuit.circuitGates_[i].atpgVal_;
			if (originalVal != X)
			{
				gateID_to_valModified_[i] = 1;
				for (int j = 0; j < reinitializedCircuit.circuitGates_[i].numFO_; ++j)
				{
					gateID2scheduled[reinitializedCircuit.circuitGates_[i].fanoutVector_[j]] = true;
				}
			}
		}
	}

	for (int i = reinitializedCircuit.numPI_; i < reinitializedCircuit.numGate_; ++i)
	{
		if (gateID2scheduled[i])
		{
			gateID2scheduled[i] = false;
			Value originalVal = reinitializedCircuit.circuitGates_[i].atpgVal_;
			reinitializedCircuit.circuitGates_[i].atpgVal_ = reinitializedAtpg.evaluateGoodVal(reinitializedCircuit.circuitGates_[i]);
			if (reinitializedCircuit.circuitGates_[i].atpgVal_ != X)
			{
				gateID_to_valModified_[i] = 1;
				for (int j = 0; j < reinitializedCircuit.circuitGates_[i].numFO_; ++j)
				{
					gateID2scheduled[reinitializedCircuit.circuitGates_[i].fanoutVector_[j]] = true;
				}
			}
		}
	}

	Gate *pGateForActivation = reinitializedAtpg.getGateForFaultActivation(targetFault);
	if (((pGateForActivation->atpgVal_ == L) && (targetFault.faultType_ == Fault::STR)) ||
			((pGateForActivation->atpgVal_ == H) && (targetFault.faultType_ == Fault::STF)))
	{
		return TDF_V2_FAIL;
	}

	if (pGateForActivation->atpgVal_ != X)
	{
		reinitializedAtpg.setGateAtpgValAndRunImplication(*(pGateForActivation), X);
	}

	if (!reinitializedAtpg.xPathExists(pGateForActivation))
	{
		return TDF_V2_FAIL;
	}

	// set is at stage dtc to true because needs to initialized value from p1
	pFaultyLine = reinitializedAtpg.initializeForSinglePatternGeneration(targetFault, backwardImplicationLevel, implicationStatus, true);

	if (!pFaultyLine)
	{
		return TDF_V2_FAIL;
	}
	// set backtrace flag
	backtraceFlag = INITIAL;
	while (!Finish)
	{
		if (!reinitializedAtpg.doImplication(implicationStatus, backwardImplicationLevel))
		{
			if (reinitializedAtpg.backtrackDecisionTree_.lastNodeMarked())
			{
				++numOfBacktrack;
			}
			// abort if nnumofbacktrack hits the limit
			if (numOfBacktrack > BACKTRACK_LIMIT)
			{
				genStatus = TDF_V2_FAIL;
				Finish = true;
			}

			reinitializedAtpg.clearAllEvents();

			if (reinitializedAtpg.backtrack(backwardImplicationLevel))
			{
				backtraceFlag = INITIAL;
				implicationStatus = (backwardImplicationLevel > 0) ? BACKWARD : FORWARD;
				pLastDFrontier = NULL;
			}
			else
			{
				genStatus = TDF_V2_FAIL;
				Finish = true;
			}
			continue;
		}

		// is continuation meaningful?
		if (!reinitializedAtpg.continuationMeaningful(pLastDFrontier))
		{
			backtraceFlag = INITIAL;
		}

		// if fault signal propagated to a primary output?
		if (reinitializedAtpg.checkIfFaultHasPropagatedToPO(faultHasPropagatedToPO))
		{
			if (reinitializedAtpg.checkForUnjustifiedBoundLines())
			{
				reinitializedAtpg.findFinalObjective(backtraceFlag, faultHasPropagatedToPO, pLastDFrontier);
				reinitializedAtpg.assignAtpgValToFinalObjectiveGates();
				implicationStatus = FORWARD;
				continue;
			}
			else
			{
				reinitializedAtpg.justifyFreeLines(targetFault);
				genStatus = TDF_V2_FOUND;
				Finish = true;
			}
		}
		// not propagated to po
		else
		{
			int numGatesInDFrontier = reinitializedAtpg.countEffectiveDFrontiers(pFaultyLine);

			if (numGatesInDFrontier == 0)
			{
				if (reinitializedAtpg.backtrackDecisionTree_.lastNodeMarked())
				{
					++numOfBacktrack;
				}
				if (numOfBacktrack > BACKTRACK_LIMIT)
				{
					genStatus = TDF_V2_FAIL;
					Finish = true;
				}

				reinitializedAtpg.clearAllEvents();

				if (reinitializedAtpg.backtrack(backwardImplicationLevel))
				{
					backtraceFlag = INITIAL;
					implicationStatus = (backwardImplicationLevel > 0) ? BACKWARD : FORWARD;
					pLastDFrontier = NULL;
				}
				else
				{
					genStatus = TDF_V2_FAIL;
					Finish = true;
				}
			}
			else if (numGatesInDFrontier == 1)
			{
				// unique sensitization because there exist just on path to po
				backwardImplicationLevel = reinitializedAtpg.doUniquePathSensitization(reinitializedCircuit.circuitGates_[reinitializedAtpg.dFrontiers_[0]]);
				// unique sensitization fail
				if (backwardImplicationLevel == UNIQUE_PATH_SENSITIZE_FAIL)
				{
					continue;
				}

				if (backwardImplicationLevel > 0)
				{
					implicationStatus = BACKWARD;
					continue;
				}
				else if (backwardImplicationLevel == 0)
				{
					continue;
				}
				else
				{
					// backwardImplicationLevel < 0, find an objective and set backtraceFlag and pLastDFrontier
					reinitializedAtpg.findFinalObjective(backtraceFlag, faultHasPropagatedToPO, pLastDFrontier);
					reinitializedAtpg.assignAtpgValToFinalObjectiveGates();
					implicationStatus = FORWARD;
					continue;
				}
			}
			else
			{
				// more than one dfrontiers
				reinitializedAtpg.findFinalObjective(backtraceFlag, faultHasPropagatedToPO, pLastDFrontier);
				reinitializedAtpg.assignAtpgValToFinalObjectiveGates();
				implicationStatus = FORWARD;
				continue;
			}
		}
	}
	if (genStatus == TDF_V2_FOUND)
	{
		for (int i = 0; i < reinitializedCircuit.numPI_; ++i)
		{
			pattern.PI2_[i] = reinitializedCircuit.circuitGates_[i].atpgVal_;
			if (pattern.PI2_[i] == D)
			{
				pattern.PI2_[i] = H;
			}
			if (pattern.PI2_[i] == B)
			{
				pattern.PI2_[i] = L;
			}
		}
		for (int i = 0; i < this->pCircuit_->numGate_; ++i)
		{
			this->pCircuit_->circuitGates_[i].atpgVal_ = reinitializedCircuit.circuitGates_[i].atpgVal_;
		}
	}
	return genStatus;
}

```

As for V2 first test pattern generation(`generateSinglePatternOnTargetTDF`). We implemented in a similar way shown above. The detailed implementation can be seen in the source codes.