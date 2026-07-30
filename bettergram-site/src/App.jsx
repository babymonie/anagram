import content from './data/content.json'
import Navbar from './components/Navbar'
import Hero from './components/Hero'
import TutorialSection from './components/TutorialSection'
import Features from './components/Features'
import HubSection from './components/HubSection'
import InstallSection from './components/InstallSection'
import Faq from './components/FAQ'
import Footer from './components/Footer'

export default function App() {
  return (
    <>
      <Navbar site={content.site} />
      <Hero site={content.site} stats={content.stats} />
      <TutorialSection />
      <Features categories={content.categories} features={content.features} />
      <HubSection hub={content.hubExplainer} />
      <InstallSection steps={content.installSteps} />
      <Faq faq={content.faq} />
      <Footer site={content.site} footer={content.footer} />
    </>
  )
}
