/*
 * Copyright (C) 2015 The CyanogenMod Project
 *               2017-2019,2021 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.lineageos.settings.doze;

import android.app.Activity;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;

import androidx.appcompat.app.AlertDialog;
import androidx.fragment.app.DialogFragment;

import androidx.preference.Preference;
import androidx.preference.Preference.OnPreferenceChangeListener;
import androidx.preference.PreferenceCategory;
import androidx.preference.SwitchPreferenceCompat;

import com.android.settingslib.widget.MainSwitchPreference;
import com.android.settingslib.widget.SettingsBasePreferenceFragment;

public class DozeSettingsFragment extends SettingsBasePreferenceFragment
        implements OnPreferenceChangeListener {

    private SwitchPreferenceCompat mWakeOnGesturePreference;
    private SwitchPreferenceCompat mPickUpPreference;
    private SwitchPreferenceCompat mHandwavePreference;
    private SwitchPreferenceCompat mPocketPreference;

    private Handler mHandler = new Handler();

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        setPreferencesFromResource(R.xml.doze_settings, rootKey);

        SharedPreferences prefs = getActivity().getSharedPreferences("doze_settings",
                Activity.MODE_PRIVATE);

        boolean dozeEnabled = Utils.isDozeEnabled(getActivity());

        MainSwitchPreference switchBar = findPreference(Utils.DOZE_ENABLE);
        switchBar.setOnPreferenceChangeListener(this);
        switchBar.setChecked(dozeEnabled);

        mWakeOnGesturePreference = (SwitchPreferenceCompat) findPreference(Utils.WAKE_ON_GESTURE_KEY);
        mWakeOnGesturePreference.setEnabled(dozeEnabled);
        mWakeOnGesturePreference.setOnPreferenceChangeListener(this);

        PreferenceCategory proximitySensorCategory =
                (PreferenceCategory) getPreferenceScreen().findPreference(Utils.CATEG_PROX_SENSOR);

        mPickUpPreference = (SwitchPreferenceCompat) findPreference(Utils.GESTURE_PICK_UP_KEY);
        mPickUpPreference.setEnabled(dozeEnabled);
        mPickUpPreference.setOnPreferenceChangeListener(this);

        mHandwavePreference = (SwitchPreferenceCompat) findPreference(Utils.GESTURE_HAND_WAVE_KEY);
        mHandwavePreference.setEnabled(dozeEnabled);
        mHandwavePreference.setOnPreferenceChangeListener(this);

        mPocketPreference = (SwitchPreferenceCompat) findPreference(Utils.GESTURE_POCKET_KEY);
        mPocketPreference.setEnabled(dozeEnabled);
        mPocketPreference.setOnPreferenceChangeListener(this);

        // Hide proximity sensor related features if the device doesn't support them
        if (!Utils.getProxCheckBeforePulse(getActivity())) {
            getPreferenceScreen().removePreference(proximitySensorCategory);
        }
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        boolean isChecked = (Boolean) newValue;

        if (Utils.DOZE_ENABLE.equals(preference.getKey())) {
            Utils.enableDoze(getActivity(), isChecked);

            mWakeOnGesturePreference.setEnabled(isChecked);
            mPickUpPreference.setEnabled(isChecked);
            mHandwavePreference.setEnabled(isChecked);
            mPocketPreference.setEnabled(isChecked);
        } else {
            Utils.enableGesture(getActivity(), preference.getKey(), isChecked);
        }

        Utils.checkDozeService(getActivity());
        return true;
    }
}
