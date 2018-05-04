// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-441099.js");
	}
}
var Module;
if (!Module) Module = eval('(function() { try { return Module || {} } catch(e) { return {} } })()');
else if (ENVIRONMENT_IS_SHELL) {
}
var Runtime = {
  stackSave: function () {
  },
  alignMemory: function (quantum) { var ret = size = Math.ceil()*(quantum ? quantum : 8); return ret; }}
function allocate() {
}
function callRuntimeCallbacks(callbacks) {
    var callback = callbacks.shift();
    var func = callback.func;
    if (typeof func === 'number') {
    } else {
      func();
    }
}
var __ATINIT__    = []; // functions called during startup
function ensureInitRuntime() {
  callRuntimeCallbacks(__ATINIT__);
}
/* global initializers */ __ATINIT__.push({ func: function() { runPostSets() } });
    function __formatString() {
            switch (next) {
            }
    }
  var Browser={mainLoop:{queue:[],pause:function () {
        }},moduleContextCreatedCallbacks:[],workers:[],init:function () {
      }};
var asm = (function() {
  'use asm';
function setThrew() {
}
function runPostSets() {
}
function _main() {
}
function _free() {
}
  return { runPostSets: runPostSets};
})
();
var runPostSets = Module["runPostSets"] = asm["runPostSets"];
var i64Math = (function() { // Emscripten wrapper
  /**
   */
})();
    ensureInitRuntime();
