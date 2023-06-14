/*
Authors: Advik Chitre
Date created: 03/05/23
*/

//-------------------------------- Imports ----------------------------------------------

import React from 'react';
import { BrowserRouter as Router, Routes, Route, Navigate } from 'react-router-dom';
import Home from './pages/Home';
import Replay from './pages/Replay';
import Connected from './pages/Connected';
import AddRover from './pages/AddRover';

//-------------------------------- Main -------------------------------------------------

/*
Top Level File - Contains intra-page navigation system
*/

function App() {
	/* Set IPv4 for AWS then rebuild (AWS Learner account only active for 4h) */
	localStorage.setItem('ServerIP', '54.165.59.37');

	/* Connect pages to URL endpoints */
	return (
		<Router>
			<Routes>
				<Route exact path="/" element={<Home />} />
				<Route path="/connected" element={<Connected />} />
				<Route path="/replay" element={<Replay />} />
				<Route path="/addrover" element={<AddRover />} />
				<Route path="/" element={<Navigate replace to="/home" />} />
			</Routes>
		</Router>
	);
}

export default App;