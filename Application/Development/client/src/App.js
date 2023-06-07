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
//import './App.css';

//-------------------------------- Main -------------------------------------------------

/*
Top Level File - Contains intra-page navigation system
*/

function App() {
	/* Set IPv4 for AWS then rebuild (AWS Learner account only active for 4h) */
	localStorage.setItem('ServerIP', '44.201.77.138');

	/* Connect pages to URL endpoints */
	return (
			<Router>
				<Routes>
					<Route exact path="/" element={<Home />} />
					<Route path="/connected" element={<Connected />} />
					<Route path="/replay" element={<Replay />} />
					<Route path="/" element={<Navigate replace to="/home" />} /> {/* TODO: add error alert*/}
				</Routes>
      		</Router>
	);
}

export default App;