// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation
// comments to trigger lazy compilation comments to trigger lazy compilation

// Test that IIFEs are compilable even under lazy conditions where the enclosing
// parentheses heuristic has not been triggered.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/lazy-iife-no-parens.js");
	}
}
function f() {
  return function(){ return 0; }();
}

function g() {
  function h() {
    return function(){ return 0; }();
  }
  return h();
}

f();

g();

0, function(){}();

(function(){ 0, function(){}(); })();

0, function(){ (function(){ 0, function(){}(); })(); }();
