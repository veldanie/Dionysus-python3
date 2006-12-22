AlphaSimplex3D::	    
AlphaSimplex3D(const ::Vertex& v): alpha_(0), attached_(false)
{
	for (int i = 0; i < 4; ++i)
		if (v.cell()->vertex(i)->point() == v.point())
			Parent::add(v.cell()->vertex(i));
}

AlphaSimplex3D::	    
AlphaSimplex3D(const Edge& e)
{
    Cell_handle c = e.first;
	Parent::add(c->vertex(e.second));
	Parent::add(c->vertex(e.third));
}

AlphaSimplex3D::	    
AlphaSimplex3D(const Edge& e, const SimplexSet& simplices, Facet_circulator facet_bg)
{
    Cell_handle c = e.first;
	Parent::add(c->vertex(e.second));
	Parent::add(c->vertex(e.third));

	Facet_circulator cur = facet_bg;
	SimplexSet::const_iterator cur_iter = simplices.find(AlphaSimplex3D(*cur));
	while (cur_iter == simplices.end())
	{
		++cur; 
		cur_iter = simplices.find(AlphaSimplex3D(*cur));
	}
	RealValue min = cur_iter->alpha();
	
	VertexSet::const_iterator v = Parent::vertices().begin();
	const Point& p1 = (*v++)->point();
	const Point& p2 = (*v)->point();
	attached_ = false;

	if (facet_bg != 0) do
	{
		VertexSet::const_iterator v = Parent::vertices().begin();
		int i0 = (*cur).first->index(*v++);
		int i1 = (*cur).first->index(*v);
		int i = 6 - i0 - i1 - (*cur).second;
		Point p3 = (*cur).first->vertex(i)->point();

		cur_iter = simplices.find(AlphaSimplex3D(*cur));
		if (cur_iter == simplices.end())			// cur is infinite
		{
			++cur; continue;
		}
		
		if (CGAL::side_of_bounded_sphere(p1, p2, p3) == CGAL::ON_BOUNDED_SIDE)
			attached_ = true;
		RealValue val = cur_iter->alpha();
		if (val < min)
			min = val;
		++cur;
	} while (cur != facet_bg);

	if (attached_)
		alpha_ = min;
	else
		alpha_ = CGAL::squared_radius(p1, p2);
}

AlphaSimplex3D::	    
AlphaSimplex3D(const Facet& f)
{
    Cell_handle c = f.first;
	for (int i = 0; i < 4; ++i)
		if (i != f.second)
			Parent::add(c->vertex(i));
}

AlphaSimplex3D::	    
AlphaSimplex3D(const Facet& f, const SimplexSet& simplices)
{
    Cell_handle c = f.first;
	for (int i = 0; i < 4; ++i)
		if (i != f.second)
			Parent::add(c->vertex(i));

	Cell_handle o = c->neighbor(f.second);
	int oi = o->index(c);

	VertexSet::const_iterator v = Parent::vertices().begin();
	const Point& p1 = (*v++)->point();
	const Point& p2 = (*v++)->point();
	const Point& p3 = (*v)->point();
	
	attached_ = false;
	if (CGAL::side_of_bounded_sphere(p1, p2, p3,
									 c->vertex(f.second)->point()) == CGAL::ON_BOUNDED_SIDE)
		attached_ = true;
	else if (CGAL::side_of_bounded_sphere(p1, p2, p3,
										  o->vertex(oi)->point()) == CGAL::ON_BOUNDED_SIDE)
		attached_ = true;
	else
		alpha_ = squared_radius(p1, p2, p3);
	
	if (attached_)
	{
		SimplexSet::const_iterator c_iter = simplices.find(AlphaSimplex3D(*c));
		SimplexSet::const_iterator o_iter = simplices.find(AlphaSimplex3D(*o));
		if (c_iter == simplices.end())			// c is infinite
			alpha_ = o_iter->alpha();
		else if (o_iter == simplices.end())		// o is infinite
			alpha_ = c_iter->alpha();
		else
			alpha_ = std::min(c_iter->alpha(), o_iter->alpha());
	}
}

AlphaSimplex3D::	    
AlphaSimplex3D(const Cell& c): attached_(false)
{
	for (int i = 0; i < 4; ++i)
		Parent::add(c.vertex(i));
	VertexSet::const_iterator v = Parent::vertices().begin();
	Point p1 = (*v++)->point();
	Point p2 = (*v++)->point();
	Point p3 = (*v++)->point();
	Point p4 = (*v)->point();
	alpha_ = CGAL::squared_radius(p1, p2, p3, p4);
}

bool 
AlphaSimplex3D::AlphaOrder::
operator()(const AlphaSimplex3D& first, const AlphaSimplex3D& second) const
{
	if (first.alpha() == second.alpha())
		return (first.dimension() < second.dimension());
	else
		return (first.alpha() < second.alpha()); 
}

std::ostream& 
AlphaSimplex3D::
operator<<(std::ostream& out) const
{
	for (VertexSet::const_iterator cur = Parent::vertices().begin(); cur != Parent::vertices().end(); ++cur)
		out << **cur << ", ";
	out << "value = " << value();

	return out;
}


void fill_alpha_order(const Delaunay& Dt, AlphaSimplex3DVector& alpha_order)
{
	// Compute all simplices with their alpha values and attachment information
	AlphaSimplex3D::SimplexSet simplices;
	for(Cell_iterator cur = Dt.finite_cells_begin(); cur != Dt.finite_cells_end(); ++cur)
		simplices.insert(AlphaSimplex3D(*cur));
	std::cout << "Cells inserted" << std::endl;
	for(Facet_iterator cur = Dt.finite_facets_begin(); cur != Dt.finite_facets_end(); ++cur)
		simplices.insert(AlphaSimplex3D(*cur, simplices));
	std::cout << "Facets inserted" << std::endl;
	for(Edge_iterator cur = Dt.finite_edges_begin(); cur != Dt.finite_edges_end(); ++cur)
		simplices.insert(AlphaSimplex3D(*cur, simplices, Dt.incident_facets(*cur)));
	std::cout << "Edges inserted" << std::endl;
	for(Vertex_iterator cur = Dt.finite_vertices_begin(); cur != Dt.finite_vertices_end(); ++cur)
		simplices.insert(AlphaSimplex3D(*cur));
	std::cout << "Vertices inserted" << std::endl;
    
	// Sort simplices by their alpha values
	alpha_order.resize(simplices.size());
	std::copy(simplices.begin(), simplices.end(), alpha_order.begin());
	std::sort(alpha_order.begin(), alpha_order.end(), AlphaSimplex3D::AlphaOrder());
}

