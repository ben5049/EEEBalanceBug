import React from 'react';
import { useState } from "react";
import SL_Button from '../components/Connected/SL';
import SR_Button from '../components/Connected/SR';
import L_Button from '../components/Connected/L';
import '../components/Connected/grid_Connected.css';
import '../components/grid.css';

const Connected = () => {
	const MAC = localStorage.getItem('MAC');
	const nickname = localStorage.getItem('nickname');

	const [State_Connected, setState_Connected] = useState('Start');

	console.log('CONNECTED MAC = ' + MAC)
	console.log('CONNECTED nickname = ' + nickname)
	

	return (
		<div className="background">
			<div className="wrapper">
				<div className="box-nobackground DisplayMAC_Connected">
					{nickname} / {MAC}
				</div>
				<div>
				{State_Connected == "Mapping" ? <L_Button /> : <SL_Button />}
				{State_Connected == "Mapping" ? null : <SR_Button />}
				</div>
				<div className="box Map_Connected">
					Map	
				</div>
				<div className="box Data_Connected">
					<p>Data</p><p>Data</p><p>Data&#10;Data&#10;</p>
				</div>
			</div>
		</div>
	);
};

export default Connected;
