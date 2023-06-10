#include "atpg.h"
#include <queue>
#include <stack>
#define CONFLICT 2

int ATPG::tdf_podem(const fptr fault, int &current_backtracks)
{
	int i;
	wptr wpi;														 // points to the PI currently being assigned
	forward_list<wptr> decision_tree_v1; // design_tree (a LIFO stack)
	forward_list<wptr> decision_tree_v2;
	wptr wfault;
	int attempt_num = 0; // counts the number of pattern generated so far for the given fault

	/* initialize all circuit wires to unknown */
	ncktwire = sort_wlist.size();
	ncktin = cktin.size();
	for (i = 0; i < ncktwire; i++)
	{
		sort_wlist[i]->value = U;
	}
	if (ncktin == 32)
	{
		select_fault_try = 15;
	}
	current_backtracks = 0;
	no_of_backtracks = 0;
	no_of_backtracks_v1 = 0;
	no_of_backtracks_v2 = 0;
	find_test = false;
	no_test = false;

	mark_propagate_tree(fault->node);

	new_bit = U; // new v2 bit
	last_bit = U;
	bool valid_v1 = false;
	bool valid_v2 = false;

	wptr obj_wire;
	if (fault->io)
	{
		obj_wire = fault->node->owire.front();
	}
	else
	{
		obj_wire = fault->node->iwire[fault->index];
	}

	// v1 activation
	switch (tdf_set_uniquely_implied_value(fault))
	{
		case TRUE: // if a  PI is assigned
			sim();	 // Fig 7.3
			if (fault->fault_type == obj_wire->value)
			{
				valid_v1 = true;
			}
			break;
		case CONFLICT:
			no_test = true; // cannot achieve initial objective, no test
			break;
		case FALSE:
			break; // if no PI is reached, keep on backtracing. Fig 7.A
	}

	if (!no_test) // tdf_set_uniquely_implied_value(fault) != CONFLICT
	{
		shift(); // try v2
		sim();
		switch (set_uniquely_implied_value(fault))
		{
			case TRUE: // if a  PI is assigned
				sim();	 // Fig 7.3
				wfault = fault_evaluate(fault);
				if (wfault != nullptr)
					forward_imply(wfault); // propagate fault effect
				if (check_test())
				{
					valid_v2 = true;
					if (valid_v1)
					{
						find_test = true; // v1, v2 valid, success!
															// cerr << "find v1v2\n";
					}
				}
				break;
			case CONFLICT:
				no_test = true; // cannot achieve initial objective, no test
				break;
			case FALSE:
				break; // if no PI is reached, keep on backtracing. Fig 7.A
		}
	}

	if (!valid_v1 && valid_v2)
	{
		undo_shift(); // go back to v1
		sim();
		while ((no_of_backtracks_v1 < backtrack_limit) && !no_test &&
					 !(find_test))
		{
			if (wpi = find_pi_assignment(obj_wire, fault->fault_type))
			{
				wpi->set_changed();
				decision_tree_v1.push_front(wpi);
			}
			else
			{
				while (!decision_tree_v1.empty() && (wpi == nullptr))
				{
					/* backtrack */
					if (decision_tree_v1.front()->is_all_assigned())
					{
						decision_tree_v1.front()->remove_all_assigned();
						decision_tree_v1.front()->value = U;
						decision_tree_v1.front()->set_changed();
						decision_tree_v1.pop_front();
					}
					/* flip last decision */
					else
					{
						decision_tree_v1.front()->value = decision_tree_v1.front()->value ^ 1;
						decision_tree_v1.front()->set_changed();
						decision_tree_v1.front()->set_all_assigned();
						no_of_backtracks_v1++;
						wpi = decision_tree_v1.front();
					}
				}

				// goto again; // if we want multiple patterns per fault
			}
			if (wpi != nullptr)
			{
				sim();
				if (obj_wire->value == fault->fault_type)
				{
					shift();
					sim();
					find_test = true;
				}
			}
			else
			{
				no_test = true;
			}
		}
	}

	else if (valid_v1 && !valid_v2)
	{
		wfault = fault_evaluate(fault);
		if (wfault)
		{
			forward_imply(wfault);
		}
		while ((no_of_backtracks_v2 < backtrack_limit) && !no_test &&
					 !(find_test))
		{
			/* check if test possible.   Fig. 7.1 */
			if (wpi = test_possible(fault))
			{
				wpi->set_changed();
				/* insert a new PI into decision_tree_v1 */
				decision_tree_v2.push_front(wpi);
				decision_tree_v2.front()->remove_all_assigned(true);
			}
			else
			{ // no test possible using this assignment, backtrack.

				while (!decision_tree_v2.empty() && (wpi == nullptr))
				{
					/* if both 01 already tried, backtrack. Fig.7.7 */
					if (decision_tree_v2.front()->is_all_assigned(true))
					{
						decision_tree_v2.front()->remove_all_assigned(true); // clear the ALL_ASSIGNED flag
						decision_tree_v2.front()->value = U;								 // do not assign 0 or 1
						decision_tree_v2.front()->set_changed();						 // this PI has been changed
						/* remove this PI in decision tree.  see dashed nodes in Fig 6 */
						decision_tree_v2.pop_front();
					}
					/* else, flip last decision, flag ALL_ASSIGNED. Fig. 7.8 */
					else
					{
						decision_tree_v2.front()->value = decision_tree_v2.front()->value ^ 1; // flip last decision
						decision_tree_v2.front()->set_changed();															 // this PI has been changed
						decision_tree_v2.front()->set_all_assigned(true);
						no_of_backtracks_v2++;
						wpi = decision_tree_v2.front();
					}

				} // while decision tree && ! wpi

			} // no test possible

			if (wpi)
			{
				sim();
				if (wfault = fault_evaluate(fault))
					forward_imply(wfault);
				if (check_test())
				{
					find_test = true;
				} // if check_test()
			}
			else
			{
				no_test = true;
			}
		} // while (three conditions)
	}

	else if (!valid_v1 && !valid_v2)
	{
		undo_shift();
		sim();
		while ((no_of_backtracks_v1 < backtrack_limit) && !no_test &&
					 !(find_test))
		{
			wpi = find_pi_assignment(obj_wire, fault->fault_type);
			if (wpi)
			{
				wpi->set_changed();
				decision_tree_v1.push_front(wpi);
			}
			else
			{

				while (!decision_tree_v1.empty() && (wpi == nullptr))
				{
					/* backtrack */
					if (decision_tree_v1.front()->is_all_assigned())
					{
						decision_tree_v1.front()->remove_all_assigned();
						decision_tree_v1.front()->value = U;
						decision_tree_v1.front()->set_changed();
						decision_tree_v1.pop_front();
					}
					/* flip last decision */
					else
					{
						decision_tree_v1.front()->value = decision_tree_v1.front()->value ^ 1;
						decision_tree_v1.front()->set_changed();
						decision_tree_v1.front()->set_all_assigned();
						no_of_backtracks_v1++;
						wpi = decision_tree_v1.front();
					}
				}
			}
			if (wpi)
			{
				sim();
				if (obj_wire->value == fault->fault_type)
				{
					shift();
					sim();
					wfault = fault_evaluate(fault);
					if (wfault != nullptr)
						forward_imply(wfault); // propagate fault effect
					no_of_backtracks_v2 = 0;
					while ((no_of_backtracks_v2 < backtrack_limit) && !no_test &&
								 !(find_test))
					{
						wpi = test_possible(fault);
						if (wpi)
						{
							wpi->set_changed();
							decision_tree_v2.push_front(wpi);
							decision_tree_v2.front()->remove_all_assigned(true);
						}
						else
						{
							while (!decision_tree_v2.empty() && (wpi == nullptr))
							{
								/* backtrack */
								if (decision_tree_v2.front()->is_all_assigned(true))
								{
									decision_tree_v2.front()->remove_all_assigned(true);
									decision_tree_v2.front()->value = U;
									decision_tree_v2.front()->set_changed();
									decision_tree_v2.pop_front();
								}
								/* flip last decision */
								else
								{
									decision_tree_v2.front()->value = decision_tree_v2.front()->value ^ 1;
									decision_tree_v2.front()->set_changed();
									decision_tree_v2.front()->set_all_assigned(true);
									no_of_backtracks_v2++;
									wpi = decision_tree_v2.front();
								}
							}
						}
						if (wpi)
						{
							sim();
							if (wfault = fault_evaluate(fault))
								forward_imply(wfault);
							if (check_test())
								find_test = true;
						}
						else
						{
							break;
						}
					}
					if (!find_test)
					{
						while (!decision_tree_v2.empty())
						{
							decision_tree_v2.front()->value = U;
							decision_tree_v2.front()->remove_all_assigned(true);
							decision_tree_v2.pop_front();
						}
						undo_shift();
						sim();
					}
				}
			}
			else
			{
				no_test = true;
			}
		}
	}
	/* clear everything */
	for (wptr wptr_ele : decision_tree_v1)
	{
		wptr_ele->remove_all_assigned();
	}
	decision_tree_v1.clear();
	for (wptr wptr_ele : decision_tree_v2)
	{
		wptr_ele->remove_all_assigned(true);
	}
	decision_tree_v2.clear();

	current_backtracks += no_of_backtracks;
	unmark_propagate_tree(fault->node);

	if (find_test)
	{
		if (dynamic_test_compression)
		{
			if (flist_type == 3)
			{
				tdf_podemx_bt();
			}
			else
			{
				tdf_podemx();
			}
		}

		for (i = 0; i < ncktin; i++)
		{
			switch (cktin[i]->value)
			{
				case 0:
				case 1:
					break;
				case D:
					cktin[i]->value = 1;
					break;
				case D_bar:
					cktin[i]->value = 0;
					break;
				case U:
					// cktin[i]->value = rand() & 01; // randfill at last
					break; // random fill U
			}
		}

		switch (last_bit)
		{
			case 0:
			case 1:
				break;
			case D:
				last_bit = 1;
				break;
			case D_bar:
				last_bit = 0;
				break;
			case U:
				// last_bit = rand() & 01; // randfill at last
				break; // random fill U
		}
		return (TRUE);
	}
	else if (no_test)
	{
		/*fprintf(stdout,"redundant fault...\n");*/
		return (FALSE);
	}
	else
	{
		/*fprintf(stdout,"test aborted due to backtrack limit...\n");*/
		return (MAYBE);
	}
}

int ATPG::tdf_set_uniquely_implied_value(const fptr fault)
{
	wptr w;
	int pi_is_reach = FALSE;
	int i, nin;

	nin = fault->node->iwire.size();
	if (fault->io)
	{
		w = fault->node->owire.front(); //  gate output fault, Fig.8.3
	}
	else
	{ // gate input fault.  Fig. 8.4
		w = fault->node->iwire[fault->index];
	} // else , gate input fault

	switch (backward_imply(w, (fault->fault_type)))
	{
		case TRUE:
			pi_is_reach = TRUE;
			break;
		case CONFLICT:
			return CONFLICT;
			break;
		case FALSE:
			break;
	}
	return (pi_is_reach);
}

void ATPG::shift()
{
	for (int i = 0; i < ncktin; i++)
	{
		if (sort_wlist[i]->value == D)
			sort_wlist[i]->value = 1;
		else if (sort_wlist[i]->value == D_bar)
			sort_wlist[i]->value = 0;
	}

	last_bit = cktin.back()->value;

	for (int i = ncktin - 1; i >= 1; i--)
	{
		cktin[i]->value = cktin[i - 1]->value;
	}

	cktin.front()->value = new_bit;

	for (int i = 0; i < ncktin; i++)
	{
		sort_wlist[i]->set_changed();
	}

	for (int i = ncktin; i < ncktwire; i++)
	{
		sort_wlist[i]->value = U;
	}
}

void ATPG::undo_shift()
{
	for (int i = 0; i < ncktin; i++)
	{
		if (sort_wlist[i]->value == D)
			sort_wlist[i]->value = 1;
		else if (sort_wlist[i]->value == D_bar)
			sort_wlist[i]->value = 0;
	}

	new_bit = cktin.front()->value;

	for (int i = 0; i < ncktin - 1; i++)
	{
		cktin[i]->value = cktin[i + 1]->value;
	}

	cktin.back()->value = last_bit;

	for (int i = 0; i < ncktin; i++)
	{
		sort_wlist[i]->set_changed();
	}

	for (int i = ncktin; i < ncktwire; i++)
	{
		sort_wlist[i]->value = U;
	}
}

void ATPG::tdf_podemx()
{
	int U_PO_idx = 0;
	queue<fptr> flist_sec_q;
	stack<fptr> flist_sec_s;

	switch (flist_type)
	{
		case 1:
			for (fptr fptr_ele : flist_undetect)
			{
				if (fptr_ele->detect != REDUNDANT && (fptr_ele->detected_time >= cur_i - chance))
				{
					flist_sec_q.push(fptr_ele);
				}
			}
			if (flist_sec_q.empty())
			{
				return;
			}
			break;
		case 2:
			for (fptr fptr_ele : flist_undetect)
			{
				if (fptr_ele->detect != REDUNDANT && (fptr_ele->detected_time >= cur_i - chance))
				{
					flist_sec_s.push(fptr_ele);
				}
			}
			if (flist_sec_s.empty())
			{
				return;
			}
			break;
		case 3:
			cerr << "should call tdf_podemx_bt()\n";
			return;
	}

	int continuous_fail_count = 0;
	while (continuous_fail_count < fail_continuous_limit)
	{
		// terminating condition: 1. no unknown PO,
		// 2. check unknown PO still exist
		while (U_PO_idx < cktout.size())
		{
			if (cktout[U_PO_idx]->value == U)
			{
				break;
			}
			U_PO_idx++;
		}
		if (U_PO_idx == cktout.size())
		{
			return;
		}
		fptr f_secondary = nullptr;
		switch (flist_type)
		{
			case 1:
				if (flist_sec_q.empty())
				{
					return;
				}
				f_secondary = flist_sec_q.front();
				flist_sec_q.pop();
				break;
			case 2:
				if (flist_sec_s.empty())
				{
					return;
				}
				f_secondary = flist_sec_s.top();
				flist_sec_s.pop();
				break;
			case 3:
				cerr << "should call tdf_podemx_bt()\n";
				return;
		}

		for (int i = 0; i < ncktin; i++)
		{
			switch (cktin[i]->value)
			{
				case 0:
				case 1:
				case U:
					break;
				case D:
					cktin[i]->value = 1;
					break;
				case D_bar:
					cktin[i]->value = 0;
					break;
			}
		}
		switch (last_bit)
		{
			case 0:
			case 1:
			case U:
				break;
			case D:
				last_bit = 1;
				break;
			case D_bar:
				last_bit = 0;
				break;
		}
		switch (new_bit)
		{
			case 0:
			case 1:
			case U:
				break;
			case D:
				new_bit = 1;
				break;
			case D_bar:
				new_bit = 0;
				break;
		}

		for (int i = 0; i < ncktin; ++i)
			sort_wlist[i]->set_changed();
		for (int i = ncktin; i < ncktwire; ++i)
			sort_wlist[i]->value = U;

		sim();
		switch (tdf_podemx_secondary(f_secondary))
		{
			case TRUE:
				continuous_fail_count = 0;
				break;
			default:
				continuous_fail_count++;
				break;
		}
	}
}

void ATPG::tdf_podemx_bt()
{
	if (flist_type != 3)
	{
		cerr << "should call tdf_podemx()\n";
		return;
	}
	double start_t = ((double)clock());
	int U_PO_idx = 0;
	for (fptr fptr_ele : flist_undetect)
	{
		fptr_ele->tried_dtc = false;
	}

	int continuous_fail_count = 0;
	while (continuous_fail_count < fail_continuous_limit)
	{
		// terminating condition: 1. no unknown PO,
		// 2. check unknown PO still exist
		while (U_PO_idx < cktout.size())
		{
			if (cktout[U_PO_idx]->value == U)
			{
				break;
			}
			U_PO_idx++;
		}
		if (U_PO_idx == cktout.size())
		{
			return;
		}

		fptr f_secondary = nullptr;

		// backtrace from unknown PO!
		wptr unknown_PO = cktout[U_PO_idx];
		queue<wptr> q_wire;
		queue<fptr> q_fault;
		bool PO_filled = false;

		q_wire.push(unknown_PO);
		int sl = 0;

		while (!PO_filled)
		{
			while (q_fault.empty() && sl++ < select_fault_try)
			{
				if (q_wire.empty())
				{
					break;
				}
				else
				{
					for (wptr w : q_wire.front()->inode.front()->iwire)
					{
						if (w->value == U)
						{
							q_wire.push(w);
						}
					}

					if (!q_wire.front()->udflist.empty())
					{
						for (fptr f : q_wire.front()->udflist)
						{
							if (f->detect != REDUNDANT && f->tried_dtc != true && (f->detected_time >= cur_i - chance))
							{
								q_fault.push(f);
							}
						}
					}
					q_wire.pop();
				}
			}
			if (q_fault.empty() || (select_fault_try == 0))
			{
				break; // try next unknown PO
			}
			f_secondary = q_fault.front();
			q_fault.pop();

			for (int i = 0; i < ncktin; i++)
			{
				switch (cktin[i]->value)
				{
					case 0:
					case 1:
					case U:
						break;
					case D:
						cktin[i]->value = 1;
						break;
					case D_bar:
						cktin[i]->value = 0;
						break;
				}
			}
			switch (last_bit)
			{
				case 0:
				case 1:
				case U:
					break;
				case D:
					last_bit = 1;
					break;
				case D_bar:
					last_bit = 0;
					break;
			}
			switch (new_bit)
			{
				case 0:
				case 1:
				case U:
					break;
				case D:
					new_bit = 1;
					break;
				case D_bar:
					new_bit = 0;
					break;
			}

			for (int i = 0; i < ncktin; ++i)
				sort_wlist[i]->set_changed();
			for (int i = ncktin; i < ncktwire; ++i)
				sort_wlist[i]->value = U;

			sim();
			switch (tdf_podemx_secondary(f_secondary))
			{
				case TRUE:
					if (unknown_PO->value != U)
					{
						PO_filled = true;
					}
					break;
				default:
					break;
			}
			f_secondary->tried_dtc = true;
		}

		if (unknown_PO->value == U)
		{
			U_PO_idx++;
			continuous_fail_count++;
		}
	}
}

int ATPG::tdf_podemx_secondary(const fptr fault)
{
	// v2 then v1
	int i;
	wptr wpi;													// points to the PI currently being assigned
	forward_list<wptr> decision_tree; // design_tree (a LIFO stack)
	wptr wfault;

	// store original PIs for primary fault
	vector<int> prime_PI(ncktin + 2);
	for (i = 0; i < ncktin; i++)
	{
		prime_PI[i] = cktin[i]->value;
	}
	prime_PI[ncktin] = last_bit;
	prime_PI[ncktin + 1] = new_bit;

	no_of_backtracks = 0;
	find_test = false;
	no_test = false;

	// gen v2
	mark_propagate_tree(fault->node);
	/* Fig 7 starts here */
	/* set the initial objective, assign the first PI.  Fig 7.P1 */
	switch (set_uniquely_implied_value(fault))
	{
		case TRUE: // if a  PI is assigned
			sim();	 // Fig 7.3
			wfault = fault_evaluate(fault);
			if (wfault != nullptr)
				forward_imply(wfault); // propagate fault effect
			if (check_test())
				find_test = true; // if fault effect reaches PO, done. Fig 7.10
			break;
		case CONFLICT:
			no_test = true; // cannot achieve initial objective, no test
			break;
		case FALSE:
			break; // if no PI is reached, keep on backtracing. Fig 7.A
	}

	/* loop in Fig 7.ABC
	 * quit the loop when either one of the three conditions is met:
	 * 1. number of backtracks is equal to or larger than limit
	 * 2. no_test
	 * 3. already find a test pattern AND no_of_patterns meets required total_attempt_num */
	// int dbg = 1;
	while ((no_of_backtracks < podemx_backtrack_limit) && !no_test &&
				 !(find_test))
	{
		/* check if test possible.   Fig. 7.1 */
		if (wpi = test_possible(fault))
		{
			wpi->set_changed();
			/* insert a new PI into decision_tree */
			decision_tree.push_front(wpi);
		}
		else
		{ // no test possible using this assignment, backtrack.

			while (!decision_tree.empty() && (wpi == nullptr))
			{
				/* if both 01 already tried, backtrack. Fig.7.7 */
				if (decision_tree.front()->is_all_assigned())
				{
					decision_tree.front()->remove_all_assigned(); // clear the ALL_ASSIGNED flag
					decision_tree.front()->value = U;							// do not assign 0 or 1
					decision_tree.front()->set_changed();					// this PI has been changed
					/* remove this PI in decision tree.  see dashed nodes in Fig 6 */
					decision_tree.pop_front();
				}
				/* else, flip last decision, flag ALL_ASSIGNED. Fig. 7.8 */
				else
				{
					decision_tree.front()->value = decision_tree.front()->value ^ 1; // flip last decision
					decision_tree.front()->set_changed();														 // this PI has been changed
					decision_tree.front()->set_all_assigned();
					no_of_backtracks++;
					wpi = decision_tree.front();
				}
			} // while decision tree && ! wpi
			if (wpi == nullptr)
				no_test = true; // decision tree empty,  Fig 7.9
		}										// no test possible

		/* this again loop is to generate multiple patterns for a single fault
		 * this part is NOT in the original PODEM paper  */
		// cerr << "if wpi\n";
		if (wpi)
		{
			sim();
			if (wfault = fault_evaluate(fault))
				forward_imply(wfault);
			if (check_test())
			{
				find_test = true;
			} // if check_test()
		}		// again
				// cerr << "end if\n";
	}			// while (three conditions)
	/* clear everything */
	for (wptr wptr_ele : decision_tree)
	{
		wptr_ele->remove_all_assigned();
	}
	decision_tree.clear();
	undo_shift();
	sim();
	no_of_backtracks = 0;
	// gen v1
	if (find_test)
	{
		find_test = false;
		wptr obj_wire;
		if (fault->io)
		{
			obj_wire = fault->node->owire.front();
		}
		else
		{
			obj_wire = fault->node->iwire[fault->index];
		}
		if (fault->fault_type == obj_wire->value)
		{
			find_test = true; // v1 activated + v2 detected
		}
		else
		{
			switch (tdf_set_uniquely_implied_value(fault))
			{
				case TRUE: // if a  PI is assigned
					sim();	 // Fig 7.3
					break;
				case CONFLICT:
					no_test = true; // cannot achieve initial objective, no test
					break;
				case FALSE:
					break; // if no PI is reached, keep on backtracing. Fig 7.A
			}
			while ((no_of_backtracks < podemx_backtrack_limit) && !no_test /*&& !(find_test)*/)
			{
				if (wpi = find_pi_assignment(obj_wire, fault->fault_type))
				{
					wpi->set_changed();
					decision_tree.push_front(wpi);
				}
				else
				{
					while (!decision_tree.empty() && (wpi == nullptr))
					{
						/* backtrack */
						if (decision_tree.front()->is_all_assigned())
						{
							decision_tree.front()->remove_all_assigned();
							decision_tree.front()->value = U;
							decision_tree.front()->set_changed();
							decision_tree.pop_front();
						}
						/* flip last decision */
						else
						{
							decision_tree.front()->value = decision_tree.front()->value ^ 1;
							decision_tree.front()->set_changed();
							decision_tree.front()->set_all_assigned();
							no_of_backtracks++;
							wpi = decision_tree.front();
						}
					}
					// goto again; // if we want multiple patterns per fault
				}
				if (wpi != nullptr)
				{
					sim();
					if (obj_wire->value == fault->fault_type)
					{
						find_test = true;
						break;
					}
				}
				else
				{
					break;
				}
			}
		}
	}
	for (wptr wptr_ele : decision_tree)
	{
		wptr_ele->remove_all_assigned();
	}
	decision_tree.clear();

	unmark_propagate_tree(fault->node);

	if (find_test)
	{
		shift();
		return (TRUE);
	}
	// restore prime PIs for next secondary fault
	for (i = 0; i < ncktin; i++)
	{
		cktin[i]->value = prime_PI[i];
	}
	last_bit = prime_PI[ncktin];
	new_bit = prime_PI[ncktin + 1];
	for (int i = 0; i < ncktin; i++)
	{
		sort_wlist[i]->set_changed();
	}

	for (int i = ncktin; i < ncktwire; i++)
	{
		sort_wlist[i]->value = U;
	}
	sim();
	if (no_test)
	{
		return (FALSE);
	}
	else
	{
		return (MAYBE);
	}
}