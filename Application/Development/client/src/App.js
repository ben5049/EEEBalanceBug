import React from 'react';
import './App.css';
import { BrowserRouter as Router, Routes, Route, Navigate } from 'react-router-dom';
import Home from './pages/Home';
import Replay from './pages/Replay';
import Connected from './pages/Connected';

function App() {

	// Set IPv4 for AWS then rebuild (AWS Learner account only active for 4h)
	localStorage.setItem('ServerIP', '18.207.197.24');

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
