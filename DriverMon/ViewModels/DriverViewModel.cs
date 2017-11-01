﻿using Prism.Mvvm;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DriverMon.ViewModels {
    class DriverViewModel : BindableBase {
        public string Name { get; }
        public DriverViewModel(string name) {
            Name = name;
        }

        public string DisplayName { get; set; }

        private bool _isMonitored;

        public bool IsMonitored {
            get => _isMonitored; 
            set => SetProperty(ref _isMonitored, value);
        }

    }
}
