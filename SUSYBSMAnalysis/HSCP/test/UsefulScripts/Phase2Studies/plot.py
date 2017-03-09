#! /usr/bin/env python

import os, multiprocessing
import copy
import math
from array import array
from ROOT import ROOT, gROOT, gStyle, gRandom, TSystemDirectory
from ROOT import TFile, TChain, TTree, TCut, TH1, TH1F, TH2F, TH3F, TProfile, TProfile2D, THStack, TGraph, TGraphAsymmErrors
from ROOT import TStyle, TCanvas, TPad
from ROOT import TLegend, TLatex, TText, TLine, TBox

gStyle.SetOptStat(0)
gStyle.SetOptTitle(0)
gROOT.SetBatch(True)

histos = [
    'chi2', 
    'dist', 
    'dEdXHits', 
    'pixelHits'
    'phase2sHits',
    'phase2sHitsHoT',
    'dEdXHitsVsEta',
    'pixelHitsVsEta',
    'phase2sHitsVsEta',
    'phase2sHitsHoTVsEta',
    'dEdXHitsVsPt',
    'pixelHitsVsPt',
    'phase2sHitsVsPt',
    'phase2sHitsHoTVsPt',
    'dEdXHitsVsP',
    'pixelHitsVsP',
    'phase2sHitsVsP',
    'phase2sHitsHoTVsP',
    'dEdXHitsVsTrkHits',
    'pixelHitsVsTrkHits',
    'phase2sHitsVsTrkHits',
    'phase2sHitsHoTVsTrkHits',
    'pixelLayVsEta',
    'pixelBarLayVsEta',
    'pixelEndLayVsEta',
    'phase2sLayVsEta',
    'phase2sBarLayVsEta',
    'phase2sEndLayVsEta',
    'phase2sHoTLayVsEta',
    'phase2sHoTBarLayVsEta',
    'phase2sHoTEndLayVsEta',
    'hit_PO_Hit',
    'hit_SO_in_noC_CCC_Hit',
    'hit_SP_in_noC_CCC_Hit'
]


def drawAnalysis(isSignal=True):
    analyses  = 'HSCP'
    analyses += ', signal' if isSignal else ', MinBias'
    latex = TLatex()
    latex.SetNDC()
    latex.SetTextSize(0.04)
    latex.SetTextFont(42)
    #latex.SetTextAlign(33)
    latex.DrawLatex(0.15 , 0.95, analyses)

def drawRegion(channel='tracker', left=False):
    region = {"tracker" : "Phase2 tracker"}
    
    text = ""
    if channel in region:
        text = region[channel]
    else: #if channel.startswith('X') or channel.startswith('A'):
        # leptons
        if 'MC' in channel: text += ", simulation"
        elif 'DATA' in channel: text += ", data"
#    else:
#        return False
    latex = TLatex()
    latex.SetNDC()
    latex.SetTextFont(72) #52
    latex.SetTextSize(0.035)
    if left: latex.DrawLatex(0.15, 0.75, text)
    else:
        latex.SetTextAlign(22)
        latex.DrawLatex(0.5, 0.85, text)

def drawRMS(rms=0.):

    text = ("RMS = %f") % (rms)

    latex = TLatex()
    latex.SetNDC()
    latex.SetTextFont(72) #52
    latex.SetTextSize(0.035)
    latex.SetTextAlign(22)
    latex.DrawLatex(0.5, 0.75, text)
        
def drawCMS(lumi='xxx', text='Preliminary', onTop=False):
    latex = TLatex()
    latex.SetNDC()
    latex.SetTextSize(0.04)
    latex.SetTextColor(1)
    latex.SetTextFont(42)
    latex.SetTextAlign(33)
    if (type(lumi) is float or type(lumi) is int) and float(lumi) > 0: latex.DrawLatex(0.95, 0.985, "%.1f fb^{-1}  (13 TeV)" % (float(lumi)/1000.))
    #elif type(lumi) is str: latex.DrawLatex(0.95, 0.985, "%s fb^{-1}  (13 TeV)" % lumi)
    if not onTop: latex.SetTextAlign(11)
    latex.SetTextFont(62)
    latex.SetTextSize(0.05 if len(text)>0 else 0.06)
    if not onTop: latex.DrawLatex(0.15, 0.87 if len(text)>0 else 0.84, "CMS")
    else: latex.DrawLatex(0.20, 0.99, "CMS")
    latex.SetTextSize(0.04)
    latex.SetTextFont(52)
    if not onTop: latex.DrawLatex(0.15, 0.83, text)
    else: latex.DrawLatex(0.40, 0.98, text)
        
def setHistStyle(hist, r=1.1):
    hist.GetXaxis().SetTitleSize(hist.GetXaxis().GetTitleSize()*r*r)
    hist.GetYaxis().SetTitleSize(hist.GetYaxis().GetTitleSize()*r*r)
    hist.GetXaxis().SetLabelSize(hist.GetXaxis().GetLabelSize()*r)
    hist.GetYaxis().SetLabelSize(hist.GetYaxis().GetLabelSize()*r)
    hist.GetXaxis().SetLabelOffset(hist.GetXaxis().GetLabelOffset()*r*r*r*r)
    hist.GetXaxis().SetTitleOffset(hist.GetXaxis().GetTitleOffset()*r)
    hist.GetYaxis().SetTitleOffset(hist.GetYaxis().GetTitleOffset())
    if hist.GetXaxis().GetTitle().find("GeV") != -1: # and not hist.GetXaxis().IsVariableBinSize()
        div = (hist.GetXaxis().GetXmax() - hist.GetXaxis().GetXmin()) / hist.GetXaxis().GetNbins()
        hist.GetYaxis().SetTitle("Events / %.1f GeV" % div)


##################

# analysis = ['step3', 'MinBias140']
analysis = ['MinBias140']

for a in analysis:

    f = TFile('out_'+a+'.root','OPEN')

    for h in histos:
        hist = f.Get(h)
        if not hist: continue
        c1 = TCanvas("c1", h, 800, 600)
        hist.Draw('colz')
        c1.GetPad(0).SetTopMargin(0.06)
        c1.GetPad(0).SetRightMargin(0.12 if 'Vs' in h else 0.08)
        c1.GetPad(0).SetTicks(1, 1)
        drawCMS()
        drawRegion()
        drawAnalysis(False if 'MinBias' in a else True)
        hist.SetMaximum(max(hist.GetMaximum(), hist.GetMaximum())*1.25)
        hist.SetMinimum(0.)
        #setHistStyle(hist,1.1)
        c1.SaveAs('plots/'+a+'/'+h+'.png')
        c1.SaveAs('plots/'+a+'/'+h+'.pdf')

        #####################

    hist = f.Get('Ias_SP_HitProfile')
    RMSs = []
    HISTs= []
    theRMS = TH2F('theRMS','theRMS',10,0,10,10,0,10)
    for ipix in range(0,20):
        for istr in range(0,20):
            c2 = TCanvas("c2", "c2", 800, 600)
            p = hist.ProjectionZ("dEdX_"+str(ipix)+"_"+str(istr),ipix,60,istr,60)
            p.Draw()
            p.GetXaxis().SetTitle("dE/dX")
            p.Draw('colz')
            c1.GetPad(0).SetTopMargin(0.06)
            c1.GetPad(0).SetRightMargin(0.08)
            c1.GetPad(0).SetTicks(1, 1)
            drawCMS()
            drawRegion()
            drawAnalysis(False if 'MinBias' in a else True)
            p.SetMaximum(max(p.GetMaximum(), p.GetMaximum())*1.25)
            p.SetMinimum(0.)
            drawRMS(p.GetRMS())
            theRMS.SetBinContent(ipix,istr,p.GetRMS())
            if p.GetRMS() > 0:
                RMSs.append(p.GetRMS())
                HISTs.append(p)
            #setHistStyle(hist,1.1)
            c2.SaveAs('plots/'+a+'/'+"dEdX_"+str(ipix)+"_"+str(istr)+'.png')
            c2.SaveAs('plots/'+a+'/'+"dEdX_"+str(ipix)+"_"+str(istr)+'.pdf')

    lst  = zip(HISTs,RMSs)
    lst_ = sorted(lst, key=lambda tup: tup[1])

    for i in range(0,10):
        print lst_[i][1],lst_[i][0].GetName()

    c3 = TCanvas("c3", "c3", 800, 600)
    c3.GetPad(0).SetTopMargin(0.06)
    c3.GetPad(0).SetRightMargin(0.12)
    c3.GetPad(0).SetTicks(1, 1)
    theRMS.Draw("colz")
    theRMS.GetXaxis().SetTitle("pix hits (>)")
    theRMS.GetYaxis().SetTitle("strip hits (>)")
    drawCMS()
    drawRegion()
    drawAnalysis(False if 'MinBias' in a else True)
    c3.SaveAs('plots/'+a+'/'+"dEdX_RMS.png")
    c3.SaveAs('plots/'+a+'/'+"dEdX_RMS.pdf")
