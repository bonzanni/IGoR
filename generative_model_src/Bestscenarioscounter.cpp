/*
 * Bestscenarioscounter.cpp
 *
 *  Created on: Aug 19, 2016
 *      Author: quentin
 */

#include "Bestscenarioscounter.h"

using namespace std;

Best_scenarios_counter::Best_scenarios_counter(size_t n_scenarios): n_scenarios_counted(n_scenarios)  {

}

Best_scenarios_counter::Best_scenarios_counter() {
	// TODO Auto-generated constructor stub

}

Best_scenarios_counter::~Best_scenarios_counter() {
	// TODO Auto-generated destructor stub
}

void Best_scenarios_counter::count_scenario(double scenario_seq_joint_proba , double scenario_probability , const string& original_sequence ,  Seq_type_str_p_map& constructed_sequences , const Seq_offsets_map& seq_offsets , const unordered_map<tuple<Event_type,Gene_class,Seq_side>, shared_ptr<Rec_Event>>& events_map , Mismatch_vectors_map& mismatches_lists){

	if(this->best_scenarios_vec.size()<this->n_scenarios_counted){
		for(forward_list<shared_ptr<const Rec_Event>>::const_iterator iter = this->event_fw_list.begin() ; iter != this->event_fw_list.end() ; ++iter){
			this->single_scenario_realizations_queue.push((*iter)->get_current_realizations_index_vec());
		}
		if(this->best_scenarios_vec.empty()){
			this->best_scenarios_vec.emplace_back(scenario_seq_joint_proba,const_cast<queue<vector<int>>&>(this->single_scenario_realizations_queue));
		}
		else{
			vector<pair<double,queue<vector<int>>>>::iterator jter = this->best_scenarios_vec.begin();
			while( (scenario_seq_joint_proba>(*jter).first) and (jter != this->best_scenarios_vec.end()) ){
				++jter;
			}
			this->best_scenarios_vec.emplace(jter , scenario_seq_joint_proba,const_cast<queue<vector<int>>&>(this->single_scenario_realizations_queue));
		}
	}
	else{
		if(scenario_seq_joint_proba > this->best_scenarios_vec[0].first){

			for(forward_list<shared_ptr<const Rec_Event>>::const_iterator iter = this->event_fw_list.begin() ; iter != this->event_fw_list.end() ; ++iter){
				this->single_scenario_realizations_queue.push((*iter)->get_current_realizations_index_vec());
			}

			auto jter = this->best_scenarios_vec.begin()+1;
			while( (scenario_seq_joint_proba>(*jter).first) and (jter != this->best_scenarios_vec.end()) ){
				++jter;
			}
			this->best_scenarios_vec.emplace(jter , scenario_seq_joint_proba,this->single_scenario_realizations_queue);
			this->best_scenarios_vec.erase(this->best_scenarios_vec.begin());
		}
	}

	while(not this->single_scenario_realizations_queue.empty()){
		this->single_scenario_realizations_queue.pop();
	}

}

void Best_scenarios_counter::count_sequence(double seq_likelihood , const Model_marginals& single_seq_marginals , const Model_Parms& single_seq_model_parms){
	for(vector<pair<double,queue<vector<int>>>>::iterator iter = this->best_scenarios_vec.begin() ; iter!=this->best_scenarios_vec.end() ; ++iter){
		(*iter).first/=seq_likelihood;
		//If an exception is thrown here there is a problem upstream
	}
}

void Best_scenarios_counter::initialize_counter(const Model_Parms& parms , const Model_marginals& marginals){
	if(not fstreams_created){
		this->output_scenario_file.open(path_to_file + "best_scenarios_counts.csv");
		//Create the header
		this->output_scenario_file<<"seq_index;scenario_rank;scenario_proba_cond_seq";
		auto event_queue = parms.get_model_queue();

		while(not event_queue.empty()){
			shared_ptr<Rec_Event> event_ptr = event_queue.front();
			this->output_scenario_file<<";"<<event_ptr->get_name();
			this->event_fw_list.emplace_front( event_ptr);
			event_queue.pop();
		}

		this->output_scenario_file<<endl;

		fstreams_created = true;
	}
}


void Best_scenarios_counter::dump_sequence_data(int seq_index , int iteration_n){

	size_t counter = 1;
	for(vector<pair<double,queue<vector<int>>>>::reverse_iterator iter = this->best_scenarios_vec.rbegin() ; iter!=this->best_scenarios_vec.rend() ; ++iter){
		this->output_scenario_file<<seq_index<<";"<<counter<<";"<<(*iter).first;
		queue<vector<int>>& scenario_queue = (*iter).second;
		while(not scenario_queue.empty()){
			const vector<int>& real_vec = scenario_queue.front();
			this->output_scenario_file<<"(";
			for(vector<int>::const_iterator jter = real_vec.begin() ; jter!= real_vec.end() ; ++jter){
				this->output_scenario_file<<(*jter);
				if(jter!=real_vec.end() -1){
					this->output_scenario_file<<",";
				}
			}
			this->output_scenario_file<<")";
			scenario_queue.pop();
		}
		++counter;
	}
}
