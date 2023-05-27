#include "atpg.h"

#define CONFLICT 2

int ATPG::tdf_podem(const fptr fault, int &current_backtracks)
{
	// cerr << "start tdf_podem\n";
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
	// cerr << "v1 activate\n";
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
		// cerr << "test exist\n";
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
		// cerr << "no v1\n";
		undo_shift(); // go back to v1
		// cerr << "finish unshift\n";
		sim();
		// cerr << "finish sim\n";
		while ((no_of_backtracks_v1 < backtrack_limit) && !no_test &&
					 !(find_test))
		{
			// wpi = find_pi_assignment(obj_wire, fault->fault_type);
			if (wpi = find_pi_assignment(obj_wire, fault->fault_type))
			{
				// cerr << "good wpi\n";
				wpi->set_changed();
				decision_tree_v1.push_front(wpi);
			}
			else
			{
				// cerr << "no wpi\n";

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
			// cerr << "still alive\n";
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
		// cerr << "no v2\n";
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
		// cerr << "no both\n";
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
	// cerr << "start clear\n";
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
		/* normally, we want one pattern per fault */
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
		// display_io();
		// }
		// else
		// 	fprintf(stdout, "\n"); // do not random fill when multiple patterns per fault
		// cerr << "gen: ";
		// for (i = 0; i < ncktin; i++)
		// {
		// cerr << cktin[i]->value;
		// }
		// cerr << last_bit << endl;
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
