package com.demo.test.viewmodel

import androidx.lifecycle.MutableLiveData
import com.zhifeiji.base.viewmodel.BaseViewModel

class AppViewModel : BaseViewModel() {

    var isLogin: MutableLiveData<Boolean> = MutableLiveData<Boolean>(true)


    companion object {
        private const val TAG = "AppViewModel"
    }
}